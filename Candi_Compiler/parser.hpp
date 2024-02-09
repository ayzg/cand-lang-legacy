#pragma once
#include <vector>
#include <stdexcept>
#include <typeinfo>
#include <typeindex>
#include <map>
#include "char_traits.hpp"
#include "token.hpp"
#include "tokenizer.hpp"
#include "ast_node.hpp"
#include <stack>
#include "syntax_traits.hpp"
#include "parser_utils.hpp"

namespace caoco {
	using tk_vector_t = std::vector<Tk>;
	using tk_iterator_t = tk_vector_t::const_iterator;
	using tk_enum = Tk::eType;
	using node_enum = Node::eType;

	// <@class:ParsingResult>
	// <@brief> A struct containing the result of a parsing method.
	// <@member:cursor> The end of the parsed statement.
	// <@member:valid>	If the statement is valid, error handling will be done by caller.
	// <@member:error_message>	The error message if the statement is invalid. 
	//							NOTE: Error messages are passed up a call stack through a ParsingProcess.
	struct ParsingResult {
		tk_vector_t::const_iterator it_;
		Node node_;
		bool valid_{ true };
		std::string error_message_{ "" };

		ParsingResult() = default;
		ParsingResult(Node node,tk_vector_t::const_iterator it = {}, bool valid = true, std::string error_message = "")
			: node_(node),it_(it), valid_(valid), error_message_(error_message) {}

		constexpr tk_vector_t::const_iterator it() const noexcept { return it_; }
		constexpr Node& node() noexcept { return node_; }
		constexpr bool valid() const noexcept { return valid_; }
		constexpr std::string error_message() const noexcept { return error_message_; }
	};


	// <@class:ParsingProcess> virtual base class for a parsing process functor.
	class ParsingProcess {
		std::stringstream error_stream_;
		ParsingResult make_result(Node node, tk_vector_t::const_iterator cursor, bool valid = true, std::string error_message = "") {
			error_stream_ << error_message << "\n";
			return ParsingResult{ node,cursor, valid, error_stream_.str() };
		}
	protected:
		ParsingResult make_error(tk_vector_t::const_iterator cursor, Tk offending_token, std::string error_message) {
			error_stream_ << "\n" << error_message << " Offending token: " << sl::to_str(offending_token.literal())
				<< "| Line: " << offending_token.line() << "| Col: " << offending_token.col() << "\n";
			return ParsingResult{ Node(Node::eType::invalid_),cursor, false, error_stream_.str() };
		}
		ParsingResult make_success(Node node,tk_vector_t::const_iterator end) {
			return make_result(node,end, true);
		}
		ParsingResult make_success(Node node) {
			return make_result(node, node.token_end(), true);
		}
		ParsingResult make_pass(tk_vector_t::const_iterator it) {
			return make_result(Node(Node::eType::none_),it, false);
		}

		ParsingProcess(const std::type_info & parsing_process_type) { error_stream_ << "[" << parsing_process_type.name() << "]"; };
		ParsingProcess(const ParsingProcess& other) {
			error_stream_ << other.error_stream_.str();
		};
		virtual ParsingResult perform(tk_vector_t::const_iterator begin, tk_vector_t::const_iterator end) = 0;
	public:
		ParsingResult operator()(tk_vector_t::const_iterator begin, tk_vector_t::const_iterator end) {
			return perform(begin, end);
		}
		virtual ~ParsingProcess() = default;
	};


	// Expression parsing methods:
	std::optional<Node> build_statement(std::vector<Tk>::const_iterator begin,
		std::vector<Tk>::const_iterator end, std::optional<Node> last_pass = std::nullopt) {
		TokenCursor it(begin,end);

		if (!last_pass.has_value()) {
			// Determine the following operator and first operand.
			if (it.operation() == Operation::unary_) { // This expression starts with a unary operation.
				Node unary_operation = it.to_statement();
				if (*it.next(2) == end) { // Unary operation is not followed by operand.
					unary_operation.push_back(it.next().to_statement()); // lhs of unary op is the operand of the unary op.
					return unary_operation; // Entire statement is this unary operation.
				}
				else { // Unary operation is followed by operand.
					if (it.importance() < it.next(2).importance()) { // Unary operation is less important than next operation.
						Node next_operation = it.next(2).to_statement();	// next op is the next operation.
						next_operation.push_back(it.next().to_statement());	// lhs of next op is the operand of the unary op.
						unary_operation.push_back(build_statement(*it.next(2), end, std::make_optional(next_operation)).value()); // Rest of expr is operand of unary op.
						return std::make_optional(unary_operation);	// Entire statement is a unary operation with rest of expr as the operand.
					}
					else if (it.importance() >= it.next(2).importance()) { // Unary operation is more or equally important than next operation.
						Node next_pass = it.next(2).to_statement(); // next pass is the next operation.
						unary_operation.push_back(it.next().to_statement()); // lhs of unary op is the operand of the unary op.
						next_pass.push_back(unary_operation); // lhs of next pass is the unary op.
						return build_statement(*it.next(2), end, std::make_optional(next_pass));  // Rest of expr is the next pass.
					}
				}
			}
			else if (it.type() == Tk::eType::open_scope) { // This expression starts with a scope.
				ScopeResult scope = find_scope(*it, end);
				if (!scope.valid) { // If the scope is invalid, throw an error.
					throw std::runtime_error("Mismatched parenthesis.");
				}
				else if (scope.is_empty()) { // If the scope is empty, throw an error.
					throw std::runtime_error("Empty parenthesis.");
				}
				else {
					if (scope.scope_end() == end) {	// Scope is redundant, the entire statement is the scope.
						return build_statement(scope.contained_begin(), scope.contained_end());
					}
					else { // If the scope is followed by an operator, the scope is a lhs operand.
						Node next_pass = TokenCursor(scope.scope_end(),end).to_statement(); // next pass is the following operator.
						next_pass.push_back(build_statement(scope.contained_begin(), scope.contained_end()).value()); // lhs of next pass is the scope.
						return build_statement(scope.scope_end(), end, std::make_optional(next_pass)); // Rest of expr is the next pass.
					}
				}
			}
			else { // first token is assumed to be a singular operand NOTE: (may change this later to throw an error on invalid token)
				if (*it.next() == end) { // If there is no following operator. This is the last pass.
					return it.to_statement();
				}
				else if (it.next().type_is(tk_enum::open_scope)) { // special case for function call
					// Everything within the scope is the arguments of the function call.
					// The function call is the lhs of the following operator.
					ScopeResult arg_scope = find_scope(*it.next(), end);
					if (!arg_scope.valid) {
						throw std::runtime_error("Mismatched parenthesis in arguments to function call operator.");
					}

					Node function_call = Node(Node::eType::function_call_, *it, arg_scope.scope_end());
					function_call.push_back(it.to_statement()); // lhs of function call is the this operand.
					function_call.push_back({Node::eType::arguments_,arg_scope.contained_begin(), arg_scope.contained_end()}); // rhs of function call is the arguments.

					if (arg_scope.scope_end() == end) { // If there is no following operator the entire statement is the function call.
						return function_call;
					}
					else { 
						Node first_pass = TokenCursor(arg_scope.scope_end(), end).to_statement(); // first pass is the following operator.
						first_pass.push_back(function_call); // lhs of first pass is the function call.
						return build_statement(arg_scope.scope_end(), end, first_pass); // Rest of expr is the first pass.
					}
				}
				else { // If there is a following operator. This is the first pass.
					Node first_pass = it.next().to_statement(); // first pass is the following operator.
					first_pass.push_back(it.to_statement()); // lhs of first pass is the this operand.
					return build_statement(*it.next(), end, first_pass); // Rest of expr is the first pass.
				}
			}
		}
		else {
			// This is a following pass, assume 'it' is a binary operator. 
			// Assume last_pass is an unfinished binary operation of type binary operator.
			// Do some input validation first to reduce code duplication.
			if (*it.next() == end || it.next().type_is(tk_enum::eof)) { // If there is no following operand. Error.Binary Operator must be followed by operand.
				throw std::runtime_error("End of expression after binary operator. Operator must be followed by operand.");
			}

			// We will check for single operands, if it is not. Then we assume it must be a scope or a unary operator.Otherwise Error.
			if (it.next().type() != Tk::eType::number_literal
				&& it.next().type() != Tk::eType::alnumus) { // is not a single operand?
				if (it.next().operation() == Operation::unary_) { // is a unary operator?
					if (*it.next(2) == end) {
						throw std::runtime_error("End of expression after unary operator. Operator must be followed by operand.");
					}
				}
				else if (it.next().type() == Tk::eType::open_scope) { // is a scope?
					ScopeResult scope = find_scope(*it.next(), end);
					if (!scope.valid) {
						throw std::runtime_error("Mismatched parenthesis.");
					}
					else if (scope.is_empty()) {
						throw std::runtime_error("Empty parenthesis.");
					}
				}
				else  // is not a unary operator or a scope? Error!
					throw std::runtime_error("Invalid right hand side operand. Operator must be followed by operand.");
			}

			// Determine the following operator and operand.
			// If the following operand token is a unary operation, the next operator is the one after that operation.
			// If the following operand is a scope, everything within the scope is a statement- which is the operand. 
			//		. next operator is the one after the scope.
			// Else the next operator is the token after the following operand.
			std::vector<Tk>::const_iterator next_operator_it;
			if (it.next().operation() == Operation::unary_) {
				next_operator_it = *it.next(3);
			}
			else if (it.next().type() == Tk::eType::open_scope) {
				ScopeResult scope = find_scope(*it.next(), end);
				next_operator_it = scope.scope_end();
			}
			else {
				next_operator_it = *it.next(2);
			}

			TokenCursor next_op_cursor = TokenCursor(next_operator_it, end);
			Node optional_function_call = Node(Node::eType::none_);
			// Special case for function call
			if (next_op_cursor.type_is(tk_enum::open_scope)) {
				// Operand followed by a scope is a function call.
				// Everything within the scope is the arguments of the function call.
				auto arg_scope = find_scope(*next_op_cursor, end);
				if (!arg_scope.valid) {
					throw std::runtime_error("Mismatched parenthesis in arguments to function call operator.");
				}

				auto function_call = Node(Node::eType::function_call_, *it, arg_scope.scope_end());
				function_call.push_back(it.next().to_statement());
				function_call.push_back({ Node::eType::arguments_,arg_scope.contained_begin(), arg_scope.contained_end() });

				next_operator_it = arg_scope.scope_end();
				next_op_cursor = TokenCursor(next_operator_it, end);

				optional_function_call = function_call;
			}

			// If we are at the end of the expression, this is the last pass. Complete the binary operation based on associativity and rhs operand.
			if (next_operator_it == end) {
				if (it.associativity() == Associativity::right_) { // right assoc push front next operand as lhs
					if (it.next().operation() == Operation::unary_) { // next operand is a unary operation.
						last_pass.value().push_front(it.next().to_statement());
						last_pass.value().front().push_back(it.next(2).to_statement());
					}
					else if (it.next().type() == Tk::eType::open_scope) { // next operand is a scope.
						ScopeResult scope = find_scope(*it, end);
						last_pass.value().push_front(build_statement(scope.contained_begin(), scope.contained_end()).value());
					}
					else { // next operand is a single operand.
						// if next operand is followed actually a function call use that as the rhs instead.
						if (optional_function_call.type() != Node::eType::none_) {
							last_pass.value().push_front(optional_function_call);
						}
						else
							last_pass.value().push_front(it.next().to_statement());
					}
					return last_pass;
				}
				else { // left assoc push back next operand as rhs
					if (it.next().operation() == Operation::unary_) { // next operand is a unary operation.
						last_pass.value().push_back(it.next().to_statement());
						last_pass.value().back().push_back(it.next(2).to_statement());
					}
					else if (it.next().type() == Tk::eType::open_scope) { // next operand is a scope.
						ScopeResult scope = find_scope(*it.next(), end);
						last_pass.value().push_back(build_statement(scope.contained_begin(), scope.contained_end()).value());
					}
					else { // next operand is a single operand.
						// if next operand is followed actually a function call use that as the rhs instead.
						if (optional_function_call.type() != Node::eType::none_) {
							last_pass.value().push_back(optional_function_call);
						}
						else
							last_pass.value().push_back(it.next().to_statement());
					}
					return last_pass;
				}
			}
			else {
				// Else we are inside a binary operation. Check if the following operator is more or less important.
				if (it.importance() < next_op_cursor.importance()) { // More important?
					// Split the expression in 2 parts.Solve the right side first.Set as right hand side of the left side.Finished.
					if (it.associativity() == Associativity::right_) { // right assoc swap lhs and rhs
						last_pass.value().push_front(build_statement(*it.next(), end, std::nullopt).value()); // lhs is the rest of the expression.
						return last_pass; // End of expr.
					}
					else { // left assoc
						last_pass.value().push_back(build_statement(*it.next(), end, std::nullopt).value()); // rhs is the rest of the expression.
						return last_pass; // End of expr.
					}
				}
				else if (it.importance() >= next_op_cursor.importance()) { // Less important ?
					// .	This operator is a finished expression.Solve it.
					//		Set as left hand side of next operator. Call self with next operator as the cursor.
					//		If the operator is right associative. The right and left hand side are swapped.
					Node lhs_expression = Node(last_pass.value().type()); // lhs is the last pass.
					if (it.associativity() == Associativity::right_) { // right Assoc
						if (it.next().type() == Tk::eType::open_scope) { // next operand is a scope.
							ScopeResult scope = find_scope(*it.next(), end); // find the scope.
							lhs_expression.push_back(build_statement(scope.contained_begin(), scope.contained_end()).value()); // solve and set as lhs.
							lhs_expression.push_back(last_pass.value().front()); // last operation's lhs is the rhs.
						}
						else if (it.next().operation() == Operation::unary_) { // next operand is a unary operation.
							lhs_expression.push_back(it.next().to_statement()); // this unary op is the lhs.
							lhs_expression.back().push_back(it.next(2).to_statement()); // add operand to unary op.
							lhs_expression.push_back(last_pass.value().front()); // last operation's lhs is the rhs.
						}
						else { // next operand is a single operand.
							lhs_expression.push_back(it.next().to_statement()); // this operand is the lhs.
							lhs_expression.push_back(last_pass.value().front()); // last operation's lhs is the rhs.
						}
					}
					else { // left Assoc
						if (it.next().type() == Tk::eType::open_scope) { // next operand is a scope.
							ScopeResult scope = find_scope(*it.next(), end); // find the scope.
							lhs_expression.push_back(last_pass.value().front()); // last operation's lhs is the lhs.
							lhs_expression.push_back(build_statement(scope.contained_begin(), scope.contained_end()).value()); // solve and set as rhs.

						}
						else if (it.next().operation() == Operation::unary_) { // next operand is a unary operation.
							lhs_expression.push_back(last_pass.value().front()); // last operation's lhs is the lhs.
							lhs_expression.push_back(it.next().to_statement()); // this unary op is the rhs.
							lhs_expression.back().push_back(it.next(2).to_statement()); // add operand to unary op.
						}
						else { // next operand is a single operand.
							// if next operand is followed actually a function call use that as the rhs instead.
							if (optional_function_call.type() != Node::eType::none_) {
								lhs_expression.push_back(last_pass.value().front()); // last operation's lhs is the lhs.
								lhs_expression.push_back(optional_function_call); // this operand is the rhs.
							}
							else {
								lhs_expression.push_back(last_pass.value().front()); // last operation's lhs is the lhs.
								lhs_expression.push_back(it.next().to_statement()); // this operand is the rhs.
							}
						}
					}

					Node next_pass = next_op_cursor.to_statement(); // next pass is the next operator.
					next_pass.push_back(lhs_expression); // lhs of next pass is the lhs expression.
					return build_statement(next_operator_it, end, next_pass); // Rest of expr is the next pass.
				}
			}
		}
	}


	// <@macro:caoco_PARSING_PROCESS_DEF> Defines a parsing process functor.
#define caoco_PARSING_PROCESS_DEF(p) struct p : public ParsingProcess \
{ p() : ParsingProcess(typeid(p)) {} ParsingResult perform(tk_vector_t::const_iterator begin, tk_vector_t::const_iterator end);}

	// <@macro:caoco_PARSING_PROCESS_IMPL> Defines a parsing process functor's perform method.
#define caoco_PARSING_PROCESS_IMPL(p) 	ParsingResult p::perform(tk_vector_t::const_iterator begin, tk_vector_t::const_iterator end)
	

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// Parsing Functor Definitions
// Parsing functors must be forward declared as they are used recursively.
	caoco_PARSING_PROCESS_DEF(ParsePragmaticBlock);
	caoco_PARSING_PROCESS_DEF(ParseFunctionalBlock);
	caoco_PARSING_PROCESS_DEF(ParseValueExpression);

	// ambigous statements part of a pragmatic block.
	caoco_PARSING_PROCESS_DEF(ParseIdentifierStatement);
	caoco_PARSING_PROCESS_DEF(ParseDirectiveType);
	caoco_PARSING_PROCESS_DEF(ParseDirectiveVar);
	caoco_PARSING_PROCESS_DEF(ParseDirectiveFunc);
	caoco_PARSING_PROCESS_DEF(ParseDirectiveClass);
	caoco_PARSING_PROCESS_DEF(ParseDirectiveNone);

	// Functional block statements
	caoco_PARSING_PROCESS_DEF(ParseDirectiveReturn); // <#return> <expr> <eos>
	caoco_PARSING_PROCESS_DEF(ParseDirectiveIf);
	// literals
	caoco_PARSING_PROCESS_DEF(ParseLiteral);
	caoco_PARSING_PROCESS_DEF(ParseStringLiteral);
	caoco_PARSING_PROCESS_DEF(ParseNumberLiteral);
	caoco_PARSING_PROCESS_DEF(ParseRealLiteral);
	caoco_PARSING_PROCESS_DEF(ParseAlnumusLiteral);

	caoco_PARSING_PROCESS_DEF(ParseUnsignedLiteral);
	caoco_PARSING_PROCESS_DEF(ParseOctetLiteral);
	caoco_PARSING_PROCESS_DEF(ParseBitLiteral);

	// candi special objects
	caoco_PARSING_PROCESS_DEF(ParseCandiSpecialObject);
	caoco_PARSING_PROCESS_DEF(ParseCsoType);
	caoco_PARSING_PROCESS_DEF(ParseCsoValue);
	caoco_PARSING_PROCESS_DEF(ParseCsoIdentity);
	caoco_PARSING_PROCESS_DEF(ParseCsoInt);
	caoco_PARSING_PROCESS_DEF(ParseCsoUint);
	caoco_PARSING_PROCESS_DEF(ParseCsoReal);
	caoco_PARSING_PROCESS_DEF(ParseCsoOctet);
	caoco_PARSING_PROCESS_DEF(ParseCsoBit);
	caoco_PARSING_PROCESS_DEF(ParseCsoPointer);
	caoco_PARSING_PROCESS_DEF(ParseCsoArray);

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// Parsing Functor Implementations
	caoco_PARSING_PROCESS_IMPL(ParseCsoType) {
		assert(begin->type() == tk_enum::atype_ && "[PROGRAMMER LOGIC ERROR][ParseCsoType] begin is not atype_ token.");
		return make_success({ Node::eType::atype_, begin, std::next(begin) });
	}
	caoco_PARSING_PROCESS_IMPL(ParseCsoValue) {
		assert(begin->type() == Tk::eType::avalue_ && "[LOGIC ERROR][ParseCsoValue] begin is not avalue_ token.");
		return make_success({ Node::eType::avalue_, begin, std::next(begin) });
	}
	caoco_PARSING_PROCESS_IMPL(ParseCsoIdentity) {
		assert(begin->type() == Tk::eType::aidentity_ && "[LOGIC ERROR][ParseCsoIdentity] begin is not aid_ token.");
		return make_success({ Node::eType::aidentity_, begin, std::next(begin) });
	}

	caoco_PARSING_PROCESS_IMPL(ParseCsoInt) {
		assert(begin->type() == tk_enum::aint_ && "[LOGIC ERROR][ParseCsoInt] begin is not aint_ token.");
		TokenCursor it(begin, end);
		using constrained_int_type_mask = std::tuple<
			tk_mask<tk_enum::aint_>, tk_mask<tk_enum::open_frame>, tk_mask<tk_enum::subtraction, mask_policy::optional>,
			tk_mask<tk_enum::number_literal>, tk_mask<tk_enum::ellipsis>, tk_mask<tk_enum::subtraction, mask_policy::optional>,
			tk_mask<tk_enum::number_literal>, tk_mask<tk_enum::close_frame>>;
		// If '&int' is followed by an open frame '[' - it is a constrained integer type.
		// Format:
		// <constrained_int_type> 
		//		::= <aint> <open_frame> <unary_minus?> <number_literal>> <elipsis> <unary_minus?> <number_literal> <close_frame>
		//		::= <aint>
		// Example:		
		//	&int[-42...42]
		if (scan_tokens_pack<constrained_int_type_mask>(begin, end)) {
			Node atype_node(Node::eType::aint_);
			if (it.next(2).type_is(tk_enum::subtraction)) {// If the first number is negative.
				auto& unary_minus = atype_node.push_back(it.next(2).to_statement()); // Add the unary minus to the node.
				unary_minus.push_back(it.next(3).to_statement()); // Add the number to the unary minus
				// Skip the ellipsis.
				if (it.next(5).type_is(tk_enum::subtraction)) { // If the second number is negative.
									auto& unary_minus = atype_node.push_back(it.next(5).to_statement()); // Add the unary minus to the node.
									unary_minus.push_back(it.next(6).to_statement()); // Add the number to the unary minus
									// Skip the closed frame
									return make_success(atype_node, *it.next(8));
				}
				else {
					atype_node.push_back(it.next(5).to_statement()); // Add the number to the node.
					// Skip the closed frame.
					return make_success(atype_node, *it.next(7));
				}
			}
			else {
				atype_node.push_back(it.next(2).to_statement()); // Add the number to the node.
				// Skip the ellipsis.
				if (it.next(4).type_is(tk_enum::subtraction)) { // If the second number is negative.
					auto& unary_minus = atype_node.push_back(it.next(4).to_statement()); // Add the unary minus to the node.
					unary_minus.push_back(it.next(5).to_statement()); // Add the number to the unary minus
					// Skip the closed frame
					return make_success(atype_node, *it.next(7));
				}
				else {
					atype_node.push_back(it.next(4).to_statement()); // Add the number to the node.
					// Skip the closed frame.
					return make_success(atype_node, *it.next(6));
				}
			}
		}
		else {
			return make_success({ node_enum::aint_, begin, std::next(begin) });
		}
	}
	caoco_PARSING_PROCESS_IMPL(ParseCsoUint) {
		assert(begin->type() == tk_enum::auint_ && "[LOGIC ERROR][ParseCsoUint] begin is not auint_ token.");
		TokenCursor it(begin, end);
		using constrained_uint_type_mask = std::tuple<
			tk_mask<tk_enum::auint_>, tk_mask<tk_enum::open_frame>,	tk_mask<tk_enum::number_literal>, 
			tk_mask<tk_enum::ellipsis>,tk_mask<tk_enum::number_literal>, tk_mask<tk_enum::close_frame>>;
		if (scan_tokens_pack<constrained_uint_type_mask>(begin, end)) {
			Node atype_node(Node::eType::auint_);
			atype_node.push_back(it.next(2).to_statement()); // Add the number to the node.
			// Skip the ellipsis.
			atype_node.push_back(it.next(4).to_statement()); // Add the number to the node.
			// Skip the closed frame.
			return make_success(atype_node, *it.next(6));
		}
		else {
			return make_success(node_enum::auint_, *it.next());
		}
	}
	caoco_PARSING_PROCESS_IMPL(ParseCsoReal) {
		assert(begin->type() == tk_enum::areal_ && "[LOGIC ERROR][ParseCsoReal] begin is not areal_ token.");
		return make_success({ Node::eType::areal_, begin, std::next(begin) });
	}
	caoco_PARSING_PROCESS_IMPL(ParseCsoOctet) {
		assert(begin->type() == tk_enum::aoctet_ && "[LOGIC ERROR][ParseCsoOctet] begin is not aoctet_ token.");
		return make_success({ Node::eType::aoctet_, begin, std::next(begin) });
	}
	caoco_PARSING_PROCESS_IMPL(ParseCsoBit) {
		assert(begin->type() == tk_enum::abit_ && "[LOGIC ERROR][ParseCsoBit] begin is not abit_ token.");
		return make_success({ Node::eType::abit_, begin, std::next(begin) });
	}
	caoco_PARSING_PROCESS_IMPL(ParseCsoPointer) {
		assert(begin->type() == tk_enum::apointer_ && "[LOGIC ERROR][ParseCsoInt] begin is not apointer_ token.");
		TokenCursor it(begin, end);
		if (find_forward(*it, { tk_enum::apointer_, Tk::eType::open_frame ,Tk::eType::alnumus,tk_enum::close_frame})) {
			Node node(Node::eType::apointer_);
			node.push_back(it.next(2).to_statement());
			return make_success(node, *it.next(4));
		}
		else if (find_forward(*it, { tk_enum::apointer_, Tk::eType::open_frame })) { // if contraint is not an alnumus it must be a cso type.
			Node node(Node::eType::apointer_);
			auto parse_result = ParseCandiSpecialObject()(*it.next(2), it.end());

			if (parse_result.valid()) {
				node.push_back(parse_result.node());
				// Skip the close frame.
				return make_success(node, parse_result.it()+1);
			}
			else {
				throw std::runtime_error("ParseCsoPointer: Invalid CSO Type in &pointer type constraint.");
			}
		}
		else {
			throw std::runtime_error("ParseCsoPointer: Pointer must be contrained to a type.");
		}
	}
	caoco_PARSING_PROCESS_IMPL(ParseCsoArray) {
		assert(begin->type() == tk_enum::aarray_ && "[LOGIC ERROR][ParseCsoArray] begin is not aarray_ token.");
		TokenCursor it(begin, end);
		if (find_forward(*it, { tk_enum::aarray_,tk_enum::open_frame,Tk::eType::alnumus })) {
			Node node(Node::eType::aarray_);
			node.push_back(it.next(2).to_statement());

			// Should be followed by a comma and a number literal.
			if (find_forward(*it.next(3), { tk_enum::comma,tk_enum::number_literal })) {
				node.push_back(it.next(4).to_statement());
				// Skip the close frame.
				return make_success(node, *it.next(6));
			}
			else {
				throw std::runtime_error("ParseCsoArray: aarray_ must have a value constraint. Invalid array size.");
			}
		}
		else if (find_forward(*it, { tk_enum::aarray_,tk_enum::open_frame })) {
			// if constraint is not an alnumus it must be a cso type.
			Node node(Node::eType::aarray_);
			auto parse_result = ParseCandiSpecialObject()(*it.next(2), it.end());

			if (parse_result.valid()) {
				node.push_back(parse_result.node());
			
				// Should be followed by a comma and a number literal.
				if (find_forward(parse_result.it(), {tk_enum::comma,tk_enum::number_literal})) {
					//skip comma
					node.push_back(TokenCursor(parse_result.it()+1,end).to_statement());
					// Skip the close frame.
					return make_success(node, parse_result.it() + 3);
				}
				else {
					throw std::runtime_error("ParseCsoArray: aarray_ must have a value constraint. Invalid array size.");
				}
			}
			else {
				throw std::runtime_error("ParseCsoArray: Invalid CSO Type in &array type constraint.");
			}
		}
		else {
			throw std::runtime_error("ParseCsoArray: aarray_ must be contrained to a type.");
		}
	}
	caoco_PARSING_PROCESS_IMPL(ParseCandiSpecialObject) {
		switch (begin->type())
		{
		case tk_enum::atype_:
			return ParseCsoType()(begin, end);
			break;
		case tk_enum::avalue_:
			return ParseCsoValue()(begin, end);
			break;
		case tk_enum::aidentity_:
			return ParseCsoIdentity()(begin, end);
			break;
		case tk_enum::aint_:
			return ParseCsoInt()(begin, end);
			break;
		case tk_enum::auint_:
			return ParseCsoUint()(begin, end);
			break;
		case tk_enum::areal_:
			return ParseCsoReal()(begin, end);
			break;
		case tk_enum::aoctet_:
			return ParseCsoOctet()(begin, end);
			break;
		case tk_enum::abit_:
			return ParseCsoBit()(begin, end);
			break;
		case tk_enum::apointer_:
			return ParseCsoPointer()(begin, end);
			break;
		default:
			return make_error(begin, *begin, "ParseCandiSpecialObject: Invalid CSO.");
			break;
		}
	}

	caoco_PARSING_PROCESS_IMPL(ParseStringLiteral) {
		assert(begin->type() == Tk::eType::string_literal && "[LOGIC ERROR][ParseStringLiteral] begin is not string_literal token.");
		return make_success({ Node::eType::string_literal_, begin, std::next(begin) });
	}
	caoco_PARSING_PROCESS_IMPL(ParseNumberLiteral) {
		assert(begin->type() == Tk::eType::number_literal && "[LOGIC ERROR][ParseNumberLiteral] begin is not number_literal token.");
		return make_success({ Node::eType::number_literal_, begin, std::next(begin) });
	}
	caoco_PARSING_PROCESS_IMPL(ParseRealLiteral) {
		assert(begin->type() == Tk::eType::real_literal && "[LOGIC ERROR][ParseRealLiteral] begin is not real_literal token.");
		return make_success({ Node::eType::real_literal_, begin, std::next(begin) });
	}
	caoco_PARSING_PROCESS_IMPL(ParseAlnumusLiteral) {
		assert(begin->type() == Tk::eType::alnumus && "[LOGIC ERROR][ParseAlnumusLiteral] begin is not alnumus token.");
		return make_success({ Node::eType::alnumus_, begin, std::next(begin) });
	}

	caoco_PARSING_PROCESS_IMPL(ParseUnsignedLiteral){
		assert(begin->type() == tk_enum::unsigned_literal && "[LOGIC ERROR][ParseUnsignedLiteral] begin is not unsigned_literal token.");
		return make_success({ Node::eType::unsigned_literal_, begin, std::next(begin) });
	}
	caoco_PARSING_PROCESS_IMPL(ParseOctetLiteral) {
		assert(begin->type() == tk_enum::octet_literal && "[LOGIC ERROR][ParseOctetLiteral] begin is not octet_literal token.");
		return make_success({ Node::eType::octet_literal_, begin, std::next(begin) });
	}
	caoco_PARSING_PROCESS_IMPL(ParseBitLiteral) {
		assert(begin->type() == tk_enum::bit_literal && "[LOGIC ERROR][ParseBitLiteral] begin is not bit_literal token.");
		return make_success({ Node::eType::bit_literal_, begin, std::next(begin) });
	}

	caoco_PARSING_PROCESS_IMPL(ParseLiteral) {
		switch (begin->type())
		{
		case Tk::eType::string_literal:
			return ParseStringLiteral()(begin, end);
			break;
		case Tk::eType::number_literal:
			return ParseNumberLiteral()(begin, end);
			break;
		case Tk::eType::real_literal:
			return ParseRealLiteral()(begin, end);
			break;
		case Tk::eType::alnumus:
			return ParseAlnumusLiteral()(begin, end);
			break;
		case Tk::eType::none_literal_:
			return ParseDirectiveNone()(begin,end);
			break;
		case Tk::eType::unsigned_literal:
			return ParseUnsignedLiteral()(begin, end);
			break;
		case Tk::eType::octet_literal:
			return ParseOctetLiteral()(begin, end);
			break;
		case Tk::eType::bit_literal:
			return ParseBitLiteral()(begin, end);
			break;
		default:
			return make_error(begin, *begin, "ParseLiteral: Invalid literal.");
			break;
		}
	}

	caoco_PARSING_PROCESS_IMPL(ParseDirectiveType) {
		TokenCursor cursor(begin, end);
		// #type <alnumus> = <alnumus> <eos>
		if (begin->type() != Tk::eType::type_) {
			return make_error(begin, *begin, "ParseDirectiveType: Expected a type directive.");
		}

		cursor.advance();

		if (!cursor.type_is(Tk::eType::alnumus)) {
			return make_error(*cursor, **cursor, "ParseIdentifierStatement: Expected an alnumus.");
		}

		// For now the only format of identifier statement is a 
		// 1. variable assingment: <alnumus> <assingment> <literal|alnumus> <eos>

		// next should be an assignment token
		if (cursor.next().type() != Tk::eType::simple_assignment) {
			return make_error(*cursor.next(), **cursor.next(), "ParseIdentifierStatement: Expected a simple assignment token.");
		}


		// next is a type expression which is an alnumus or a cso type.
		if (cursor.next(2).type_is(Tk::eType::alnumus)) {
			// type expr is an alnumus
			auto alnumus_literal = ParseAlnumusLiteral()(*cursor.next(2), end);
			if (!alnumus_literal.valid()) {
				return make_error(*cursor.next(2), **cursor.next(2), "ParseIdentifierStatement: Expected an alnumus.");
			}

			// next should be an eos
			if (cursor.next(3).type() != Tk::eType::eos) {
				return make_error(*cursor.next(3), **cursor.next(3), "ParseIdentifierStatement: Expected an eos.");
			}

			Node node{ Node::eType::type_definition_, begin, *cursor.next(3) };
			node.push_back(Node{ Node::eType::alnumus_, *cursor, *cursor.next() });
			node.push_back(Node{ Node::eType::simple_assignment_, *cursor.next(),*cursor.next(2) });
			node.push_back(Node{ Node::eType::aint_,*cursor.next(2),alnumus_literal.it() });
			// skip eos
			return make_success(node, alnumus_literal.it() + 2);
		}
		else { // Try to parse a cso
			auto cso = ParseCandiSpecialObject()(*cursor.next(2), end);

			if (!cso.valid()) {
				// Could not parse a cso, not a valid type expr
				return make_error(*cursor.next(2), **cursor.next(2), "ParseIdentifierStatement: Expected a type expression.");
			}

			// next should be an eos
			if (cso.it()->type() != Tk::eType::eos) {
				return make_error(*cursor.next(3), **cursor.next(3), "ParseIdentifierStatement: Expected an eos.");
			}

			Node node{ Node::eType::type_definition_, begin, cso.it() + 1 };
			node.push_back(Node{ Node::eType::alnumus_, *cursor, *cursor.next() });
			node.push_back(Node{ Node::eType::simple_assignment_, *cursor.next(),*cursor.next(2) });
			node.push_back(cso.node());
			// skip eos
			return make_success(node, cso.it() + 2);
		}


		// Code should never reach here.
		assert(false && "[LOGIC ERROR][ParseDirectiveType] Code should never reach here.");
		// Create the node, omit the eos token.
		//return make_success(node, std::next(it, 4));

	}
	caoco_PARSING_PROCESS_IMPL(ParseDirectiveVar) {
		TokenCursor cursor(begin, end);

		if (find_forward(*cursor, { tk_enum::var_,tk_enum::alnumus,tk_enum::eos })) {	// Anon Var Decl
			Node node{ Node::eType::anon_variable_definition_, begin, *cursor.next(3) };
			node.push_back({ Node::eType::alnumus_,*cursor.next(), *cursor.next(2) });
			return make_success(node, *cursor.next(3));// 1 past eos token
		}
		else if (find_forward(*cursor, { tk_enum::var_,tk_enum::alnumus,tk_enum::simple_assignment })) // Anon Var Decl Assign
		{
			// Build the assingment expression
			auto expr = ParseValueExpression()(*cursor.next(), end);
			if (!expr.valid()) {
				return make_error(*cursor.next(), **cursor.next(3), "ParseDirectiveVar: Invalid var statement format. Assingment expression is invalid:" + expr.error_message());
			}
			// Create the node, omit the eos token.
			Node node{ Node::eType::anon_variable_definition_assingment_, *cursor, expr.it()-1 };
			node.push_back({ Node::eType::alnumus_, *cursor.next(), *cursor.next(2) });
			//node.push_back({ Node::eType::simple_assignment_, *cursor.next(2),*cursor.next(3) });


			node.push_back(expr.node());
			return make_success(node, expr.it()); // 1 past eos token
		}
		else if (find_forward(begin, { tk_enum::var_,tk_enum::open_frame })) { // Constrained Variable Def
			// Find the scope of the frame.
			ScopeResult frame_scope = find_statement(tk_enum::open_frame, tk_enum::close_frame, *cursor.next(), cursor.end());

			// After the frame scope must be an alnumus.
			if (find_forward(frame_scope.scope_end(), { tk_enum::alnumus })) {

				if ((frame_scope.scope_end() + 1)->type_is(tk_enum::eos)) {
					// Create the node, omit the eos token.
					Node node{ Node::eType::constrained_variable_definition_, *cursor, frame_scope.scope_end() + 1 };
					node.push_back({ Node::eType::type_constraints_, frame_scope.contained_begin(), frame_scope.contained_end() });
					node.push_back({ Node::eType::alnumus_, frame_scope.scope_end(), frame_scope.scope_end() + 1 });
					return make_success(node, frame_scope.scope_end() + 2); // 1 past eos token
				}
				else if ((frame_scope.scope_end() + 1)->type_is(tk_enum::simple_assignment)) {
					// Build the assingment expression
					auto expr = ParseValueExpression()(frame_scope.scope_end() + 2, end);
					if (!expr.valid()) {
						return make_error(*cursor.next(3), **cursor.next(3), "ParseDirectiveVar: Invalid var statement format.");
					}

					// Create the node, omit the eos token.
					Node node{ Node::eType::constrained_variable_definition_, *cursor, expr.it()-1};
					node.push_back({ Node::eType::type_constraints_, frame_scope.contained_begin(), frame_scope.contained_end() });
					node.push_back({ Node::eType::alnumus_, frame_scope.scope_end(), frame_scope.scope_end() + 1 });
					node.push_back({ Node::eType::simple_assignment_, (frame_scope.scope_end() + 1),(frame_scope.scope_end() + 2) });
					node.push_back(expr.node());
					return make_success(node, expr.it()); // 1 past eos token
				}
				else {
					return make_error(frame_scope.scope_end(), *frame_scope.scope_end(), "ParseDirectiveVar: Invalid var statement format.");
				}
			}
			else {
				return make_error(frame_scope.scope_end(), *frame_scope.scope_end(), "ParseDirectiveVar: Invalid var statement format.");
			}
		}
		else {
			return make_error(begin, *(begin+1), "ParseDirectiveVar: Invalid var statement format. #var directive was not followed by an identity or type constraint.");
		}
	}
	caoco_PARSING_PROCESS_IMPL(ParseDirectiveFunc) {
		TokenCursor cursor(begin, end);

		// Shorthand Void Arg Method Definition <#func> <alnumus> <functional_block>
		if (find_forward(*cursor, { tk_enum::func_,tk_enum::alnumus,tk_enum::open_list })) {	// Shorthand Void Arg Method (#func name {} )same as #func [@] name () {}
			ScopeResult method_def_body_scope = find_statement(tk_enum::open_list, tk_enum::close_list, *cursor.next(2), cursor.end());

			// After list scope must be an eos.
			if (method_def_body_scope.scope_end()->type_is(tk_enum::eos)) {
				// Create the node, omit the eos token.
				Node node{ Node::eType::shorthand_void_method_definition_, *cursor, method_def_body_scope.scope_end() + 2 };
				node.push_back({ Node::eType::alnumus_, *cursor.next(), *cursor.next(2) });
				node.push_back(ParseFunctionalBlock()(method_def_body_scope.contained_begin(), method_def_body_scope.contained_end()).node());
				return make_success(node, method_def_body_scope.scope_end() + 1); // 1 past eos token
			}
			else {
				return make_error(method_def_body_scope.scope_end(), *method_def_body_scope.scope_end(),
					"[ParseDirectiveFunc]: Invalid func statement format. Expected an eos token ';' after functional block in function definition. Function name:"
					+ sl::to_str(cursor.next().lit()));
			}
		}
		// Unconstrained Method Definition <#func> <alnumus> <arguments> <functional_block>
		else if (find_forward(*cursor, { tk_enum::func_,tk_enum::alnumus,tk_enum::open_scope })) {
			ScopeResult method_arguments_scope = find_scope(*cursor.next(2), cursor.end());

			if (method_arguments_scope.scope_end()->type_is(tk_enum::open_list)) {
				ScopeResult method_def_body_scope = find_statement(tk_enum::open_list, tk_enum::close_list, method_arguments_scope.scope_end(), cursor.end());
				// After list scope must be an eos.
				if (method_def_body_scope.scope_end()->type_is(tk_enum::eos)) {
					// Create the node, omit the eos token.
					Node node{ Node::eType::method_definition_, *cursor, method_def_body_scope.scope_end() + 2 };
					node.push_back({ Node::eType::alnumus_, *cursor.next(), *cursor.next(2) });
					node.push_back({ Node::eType::arguments_, method_arguments_scope.contained_begin(), method_arguments_scope.contained_end() });
					node.push_back(ParseFunctionalBlock()(method_def_body_scope.contained_begin(), method_def_body_scope.contained_end()).node());
					return make_success(node, method_def_body_scope.scope_end() + 1); // 1 past eos token
				}
				else {
					return make_error(method_def_body_scope.scope_end(), *method_def_body_scope.scope_end(), "ParseDirectiveFunc: Invalid func statement format.");
				}

			}
			else {
				return make_error(method_arguments_scope.scope_end(), *method_arguments_scope.scope_end(), "ParseDirectiveFunc: Invalid func statement format.");
			}

		}
		// Shorthand Void Constrained Method Def or Constrained Method Def
		else if (find_forward(begin, { tk_enum::func_,tk_enum::open_frame })) {
			// Find the scope of the frame.
			ScopeResult frame_scope = find_statement(tk_enum::open_frame, tk_enum::close_frame, *cursor.next(), cursor.end());

			// After the frame scope must be an alnumus and an open list -> Shorthand Void Constrained Function Def
			if (find_forward(frame_scope.scope_end(), { tk_enum::alnumus,tk_enum::open_list })) {
				ScopeResult method_def_body_scope = find_statement(tk_enum::open_list, tk_enum::close_list, frame_scope.scope_end() + 1, cursor.end());
				// After list scope must be an eos.
				if (method_def_body_scope.scope_end()->type_is(tk_enum::eos)) {
					// Create the node, omit the eos token.
					Node node{ Node::eType::shorthand_constrained_void_method_definition_, *cursor, method_def_body_scope.scope_end() + 2 };
					node.push_back({ Node::eType::type_constraints_, frame_scope.contained_begin(), frame_scope.contained_end() });
					node.push_back({ Node::eType::alnumus_, frame_scope.scope_end(), frame_scope.scope_end() + 1 });
					node.push_back({ Node::eType::functional_block_, method_def_body_scope.contained_begin(), method_def_body_scope.contained_end() });
					return make_success(node, method_def_body_scope.scope_end() + 1); // 1 past eos token
				}
				else {
					return make_error(method_def_body_scope.scope_end(), *method_def_body_scope.scope_end(), "ParseDirectiveFunc: Invalid func statement format.");
				}
			}
			// Or an alnumus and an open scope -> Constrained Method Def
			else if (find_forward(frame_scope.scope_end(), { tk_enum::alnumus,tk_enum::open_scope })) {
				ScopeResult method_arguments_scope = find_scope(frame_scope.scope_end() + 1, cursor.end());

				if (method_arguments_scope.scope_end()->type_is(tk_enum::open_list)) {
					ScopeResult method_def_body_scope = find_statement(tk_enum::open_list, tk_enum::close_list, method_arguments_scope.scope_end(), cursor.end());
					// After list scope must be an eos.
					if (method_def_body_scope.scope_end()->type_is(tk_enum::eos)) {
						// Create the node, omit the eos token.
						Node node{ Node::eType::method_definition_, *cursor, method_def_body_scope.scope_end() + 2 };
						node.push_back({ Node::eType::type_constraints_, frame_scope.contained_begin(), frame_scope.contained_end() });
						node.push_back({ Node::eType::alnumus_, frame_scope.scope_end(), frame_scope.scope_end() + 1 });
						node.push_back({ Node::eType::arguments_, method_arguments_scope.contained_begin(), method_arguments_scope.contained_end() });
						node.push_back({ Node::eType::functional_block_, method_def_body_scope.contained_begin(), method_def_body_scope.contained_end() });
						return make_success(node, method_def_body_scope.scope_end() + 1); // 1 past eos token
					}
					else {
						return make_error(method_def_body_scope.scope_end(), *method_def_body_scope.scope_end(), "ParseDirectiveFunc: Invalid func statement format.");
					}

				}
				else {
					return make_error(method_arguments_scope.scope_end(), *method_arguments_scope.scope_end(), "ParseDirectiveFunc: Invalid func statement format.");
				}
			}
			else {
				return make_error(frame_scope.scope_end(), *frame_scope.scope_end(), "ParseDirectiveVar: Invalid var statement format.");
			}
		}
		else {
			return make_error(begin, *begin, "ParseDirectiveFunc: Invalid func statement format.");
		}
	};
	caoco_PARSING_PROCESS_IMPL(ParseDirectiveClass) {
		if (begin->type() != Tk::eType::class_) {
			return make_error(begin, *begin, "ParseDirectiveClass: Expected a class directive.");
		}

		auto it = begin + 1;

		// next should be an alnumus, the class name.
		if (it->type() != Tk::eType::alnumus) {
			return make_error(it, *it, "ParseDirectiveClass: Expected an alnumus.");
		}

		// next should be an open list scope.
		if (std::next(it)->type() != Tk::eType::open_list) {
			return make_error(std::next(it), *std::next(it), "ParseDirectiveClass: Expected an open list scope.");
		}

		// Everything within the list is a pragmatic block.
		ScopeResult class_scope = find_list(std::next(it), end);
		auto class_definition = ParsePragmaticBlock()(class_scope.contained_begin(), class_scope.contained_end());
		if (!class_definition.valid()) {
			return make_error(class_definition.it(), *class_definition.it(), "ParseDirectiveClass: Invalid class definition.");
		}

		// next should be an eos
		if (class_scope.scope_end()->type() != Tk::eType::eos) {
			return make_error(class_definition.it(), *class_definition.it(), "ParseDirectiveClass: Expected an eos.");
		}

		Node node{ Node::eType::class_definition_, begin, class_scope.scope_end() };
		node.push_back(Node{ Node::eType::alnumus_, it, std::next(it) });
		node.push_back(class_definition.node());
		return make_success(node, std::next(class_scope.scope_end(), 1));
	}
	caoco_PARSING_PROCESS_IMPL(ParseDirectiveNone) {
		assert(begin->type() == Tk::eType::none_literal_ && "[LOGIC ERROR][ParseCsoValue] begin is not avalue_ token.");
		return make_success({ Node::eType::none_literal_, begin, std::next(begin) });
	}


	caoco_PARSING_PROCESS_IMPL(ParseDirectiveReturn) {
		TokenCursor cursor(begin, end);
		ScopeResult statement_scope = find_statement(Tk::eType::return_, Tk::eType::eos, begin, end);

		if (!statement_scope.valid) {
			return make_error(statement_scope.scope_end(), *statement_scope.scope_end(), "ParseDirectiveReturn: Invalid return statement.");
		}
		Node node{ Node::eType::return_, begin, statement_scope.contained_end() };
		auto& expr = node.push_back({ Node::eType::expression_, statement_scope.contained_begin(), statement_scope.contained_end() });
		expr.push_back(build_statement(statement_scope.contained_begin(), statement_scope.contained_end()).value());
		return make_success(node, statement_scope.scope_end());
	}
	caoco_PARSING_PROCESS_IMPL(ParseValueExpression) {
		auto expr_scope = caoco::find_open_statement(begin->type(), caoco::Tk::eType::eos, begin, end);

		std::optional<Node> expr_node;
		try {
			expr_node = build_statement(begin, expr_scope.contained_end());
		}
		catch (std::exception& e) {
			std::cout << e.what() << std::endl;
			return make_error(begin, *begin, "ParseValueExpression: Invalid statement.");
		}

		if(expr_node.has_value())
			return make_success(expr_node.value(), expr_scope.scope_end());
		else
			return make_error(begin, *begin, "ParseValueExpression: Invalid statement.");
	}


	caoco_PARSING_PROCESS_IMPL(ParsePragmaticBlock) {
		// Pragmatic blocks may contain statements starting with a directive or alnumus, ending in a semicolon.
		// <pragmatic_block> ::= (<directive>|<alnumus>) <statement> <eos> ?
		// <statement> ::= <type> | <var> | <func> | <class> | <identifier_statement>
		auto it = begin;
		Node node(Node::eType::pragmatic_block_, begin, end);

		// Find and parse all statements in the block.
		while (it < end && it->type() != Tk::eType::eof) {
			// Get the scope of the statement stating from the first token to the last matching semicolon.
			ScopeResult statement_scope;

			auto parse_statement = [&it,&statement_scope,&end,&node](auto parsing_process, Tk::eType open, Tk::eType close)->void {
				statement_scope = find_statement(open, close, it, end);
				// If the statement is empty, skip it.
				if (statement_scope.is_empty()) {
					std::cout << "WARNING ParsePragmaticBlock: Empty statement." << std::endl;
				}
				if (!statement_scope.valid) {
					throw std::runtime_error("ParsePragmaticBlock: Invalid statement scope.");
				}
				ParsingResult parse_result = parsing_process(statement_scope.scope_begin(), statement_scope.scope_end());
				if (!parse_result.valid()) {
					throw std::runtime_error("ParsePragmaticBlock: Invalid statement."+parse_result.error_message());
				}
				node.push_back(parse_result.node());
				it = parse_result.it();
			};
			auto parse_open_statement = [&it, &statement_scope, &end, &node](auto parsing_process, Tk::eType open, Tk::eType close)->void {
				statement_scope = find_open_statement(open, close, it, end);
				// If the statement is empty, skip it.
				if (statement_scope.is_empty()) {
					std::cout << "WARNING ParsePragmaticBlock: Empty statement." << std::endl;
				}
				if (!statement_scope.valid) {
					throw std::runtime_error("ParsePragmaticBlock: Invalid statement scope.");
				}
				ParsingResult parse_result = parsing_process(statement_scope.scope_begin(), statement_scope.scope_end());
				if (!parse_result.valid()) {
					throw std::runtime_error("ParsePragmaticBlock: Invalid statement.");
				}
				node.push_back(parse_result.node());
				it = parse_result.it();
			};

			if (it->type_is(Tk::eType::alnumus)) {
				parse_open_statement(ParseIdentifierStatement(), Tk::eType::alnumus, Tk::eType::eos);
			}
			else {
				if (it->type() == Tk::eType::type_) {
					parse_statement(ParseDirectiveType(), Tk::eType::type_, Tk::eType::eos);
				}
				else if (it->type() == Tk::eType::var_) {
					parse_statement(ParseDirectiveVar(), Tk::eType::var_, Tk::eType::eos);
				}
				else if (it->type() == Tk::eType::class_) {
					parse_statement(ParseDirectiveClass(), Tk::eType::class_, Tk::eType::eos);
				}
				else if (it->type() == Tk::eType::func_) {
					parse_statement(ParseDirectiveFunc(), Tk::eType::func_, Tk::eType::eos);
				}
				else {
					return make_error(it, *it, "ParsePragmaticBlock: Invalid statement.");
				}
			}
		}

		return make_success(node, it);
	} // end ParsePragmaticBlock
	caoco_PARSING_PROCESS_IMPL(ParseFunctionalBlock) {
		auto it = begin;
		Node node(Node::eType::functional_block_, begin, end);

		// Find and parse all statements in the block.
		while (it < end && it->type() != Tk::eType::eof) {
			// Get the scope of the statement stating from the first token to the last matching semicolon.
			ScopeResult statement_scope;

			auto parse_statement = [&it, &statement_scope, &end, &node](auto parsing_process, Tk::eType open, Tk::eType close)->void {
				statement_scope = find_statement(open, close, it, end);
				// If the statement is empty, skip it.
				if (statement_scope.is_empty()) {
					std::cout << "WARNING ParsePragmaticBlock: Empty statement." << std::endl;
				}
				if (!statement_scope.valid) {
					throw std::runtime_error("ParsePragmaticBlock: Invalid statement scope.");
				}
				ParsingResult parse_result = parsing_process(statement_scope.scope_begin(), statement_scope.scope_end());
				if (!parse_result.valid()) {
					throw std::runtime_error("ParsePragmaticBlock: Invalid statement." + parse_result.error_message());
				}
				node.push_back(parse_result.node());
				it = parse_result.it();
			};
			auto parse_open_statement = [&it, &statement_scope, &end, &node](auto parsing_process, Tk::eType open, Tk::eType close)->void {
				statement_scope = find_open_statement(open, close, it, end);
				// If the statement is empty, skip it.
				if (statement_scope.is_empty()) {
					std::cout << "WARNING ParsePragmaticBlock: Empty statement." << std::endl;
				}
				if (!statement_scope.valid) {
					throw std::runtime_error("ParsePragmaticBlock: Invalid statement scope.");
				}
				ParsingResult parse_result = parsing_process(statement_scope.scope_begin(), statement_scope.scope_end());
				if (!parse_result.valid()) {
					throw std::runtime_error("ParsePragmaticBlock: Invalid statement.");
				}
				node.push_back(parse_result.node());
				it = parse_result.it();
			};

			if (it->type_is(Tk::eType::alnumus)) {
				parse_open_statement(ParseIdentifierStatement(), Tk::eType::alnumus, Tk::eType::eos);
			}
			else {
				if (it->type() == Tk::eType::return_) {
					parse_statement(ParseDirectiveReturn(), Tk::eType::return_, Tk::eType::eos);
				}
				else if (it->type() == Tk::eType::var_) {
					parse_statement(ParseDirectiveVar(), Tk::eType::var_, Tk::eType::eos);
				}
				else {
					return make_error(it, *it, "ParsePragmaticBlock: Invalid statement.");
				}
			}
		}

		return make_success(node, it);
	}

	// Unused ?
	caoco_PARSING_PROCESS_IMPL(ParseIdentifierStatement) {
		auto it = begin;
		
		if (it->type() != Tk::eType::alnumus) {
			return make_error(it, *it, "ParseIdentifierStatement: Expected an alnumus.");
		}

		// For now the only format of identifier statement is a 
		// 1. variable assingment: <alnumus> <assingment> <literal|alnumus> <eos>

		// next should be an assignment token
		if (std::next(it)->type() != Tk::eType::simple_assignment) {
			return make_error(std::next(it), *std::next(it), "ParseIdentifierStatement: Expected a simple assignment token.");
		}

		// next should be a alnumus
		// NOTE: temporary-> this should be a value_expression
		if (std::next(it, 2)->type() != Tk::eType::number_literal) {
			return make_error(std::next(it, 2), *std::next(it, 2), "ParseIdentifierStatement: Expected an alnumus.");
		}

		// next should be an eos
		if (std::next(it, 3)->type() != Tk::eType::eos) {
			return make_error(std::next(it, 3), *std::next(it, 3), "ParseIdentifierStatement: Expected an eos.");
		}

		Node node{ Node::eType::variable_assignment_, begin, std::next(it, 3) };
		node.push_back(Node{ Node::eType::alnumus_, it, std::next(it) });
		node.push_back(Node{ Node::eType::simple_assignment_, std::next(it),std::next(it, 2) });
		node.push_back(Node{ Node::eType::number_literal_, std::next(it, 2),std::next(it, 3) });

		// Create the node, omit the eos token.
		return make_success(node, std::next(it, 4));
	}
} // end namespace caoco