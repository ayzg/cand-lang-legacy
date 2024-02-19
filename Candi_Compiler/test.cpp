#include "pch.h"
#include "unit_test_util.hpp"

#define CAOCO_TEST_ALL 1
#define CAOCO_TEST_NONE 0
#define CAOCO_TEST_TOKENIZER 1
#define CAOCO_TEST_PARSER_BASIC 1
#define CAOCO_TEST_PARSER_UTILS 1
#define CAOCO_TEST_PARSER_STATEMENTS 1
#define CAOCO_TEST_PARSER_PROGRAM 1
#define CAOCO_TEST_PREPROCESSOR 1
#define CAOCO_TEST_CONST_EVALUATOR 1

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tokenizer Tests
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#if CAOCO_TEST_TOKENIZER 
#define CAOCO_TEST_TOKENIZER_Keywords 1
#define CAOCO_TEST_TOKENIZER_KeywordsMixedShouldThrow 1
#define CAOCO_TEST_TOKENIZER_KeywordsDirectiveReportEarlyMisspell 1
#endif

#if CAOCO_TEST_TOKENIZER_Keywords
TEST(ut_Tokenizer_Keywords, ut_Tokenizer) {
	// non-directive keywords
	auto input_vec1 = caoco::sl::to_u8vec(u8"include macro enter start type var class obj private\
 public func const static if else elif while for on break continue return print none int uint real\
 byte bit str\0");
	auto expected_result1 = caoco::tk_vector({
		{caoco::tk_enum::include_},
		{caoco::tk_enum::macro_},
		{caoco::tk_enum::enter_},
		{caoco::tk_enum::start_},
		{caoco::tk_enum::type_},
		{caoco::tk_enum::var_},
		{caoco::tk_enum::class_},
		{caoco::tk_enum::obj_},
		{caoco::tk_enum::private_},
		{caoco::tk_enum::public_},
		{caoco::tk_enum::func_},
		{caoco::tk_enum::const_},
		{caoco::tk_enum::static_},
		{caoco::tk_enum::if_},
		{caoco::tk_enum::else_},
		{caoco::tk_enum::elif_},
		{caoco::tk_enum::while_},
		{caoco::tk_enum::for_},
		{caoco::tk_enum::on_},
		{caoco::tk_enum::break_},
		{caoco::tk_enum::continue_},
		{caoco::tk_enum::return_},
		{caoco::tk_enum::print_},
		{caoco::tk_enum::none_literal_},
		{caoco::tk_enum::aint_},
		{caoco::tk_enum::auint_},
		{caoco::tk_enum::areal_},
		{caoco::tk_enum::abyte_},
		{caoco::tk_enum::abit_},
		{caoco::tk_enum::astr_}
		});
	auto exp_result1 = caoco::tokenizer(input_vec1.cbegin(), input_vec1.cend())();
	if(!exp_result1.valid()){
		std::cout << exp_result1.error_message() << std::endl;
	}
	auto result1 = exp_result1.extract();
	for (auto& tk : result1) {
		std::cout << tk.type_to_string() << " ";
	}
	std::cout << std::endl;
	for (size_t i = 0; i < result1.size(); ++i) {
		EXPECT_EQ(result1[i].type(), expected_result1[i].type());
		if (result1[i].type() != expected_result1[i].type()) {
			std::cout << "Tokenization result expected keyword token: " << expected_result1[i].type_to_string()
				<< " Got: " << result1[i].type_to_string() << std::endl;
		}
	}


	// directive keywords
	auto input_vec2 = caoco::sl::to_u8vec(u8"#include #macro #enter #start #type #var #class #obj \
#private #public #func #const #static #if #else #elif #while #for #on #break #continue #return \
#print #none #int #uint #real #byte #bit #str\0");
	auto expected_result2 = caoco::tk_vector({
		{caoco::tk_enum::include_},
		{caoco::tk_enum::macro_},
		{caoco::tk_enum::enter_},
		{caoco::tk_enum::start_},
		{caoco::tk_enum::type_},
		{caoco::tk_enum::var_},
		{caoco::tk_enum::class_},
		{caoco::tk_enum::obj_},
		{caoco::tk_enum::private_},
		{caoco::tk_enum::public_},
		{caoco::tk_enum::func_},
		{caoco::tk_enum::const_},
		{caoco::tk_enum::static_},
		{caoco::tk_enum::if_},
		{caoco::tk_enum::else_},
		{caoco::tk_enum::elif_},
		{caoco::tk_enum::while_},
		{caoco::tk_enum::for_},
		{caoco::tk_enum::on_},
		{caoco::tk_enum::break_},
		{caoco::tk_enum::continue_},
		{caoco::tk_enum::return_},
		{caoco::tk_enum::print_},
		{caoco::tk_enum::none_literal_},
		{caoco::tk_enum::aint_},
		{caoco::tk_enum::auint_},
		{caoco::tk_enum::areal_},
		{caoco::tk_enum::abyte_},
		{caoco::tk_enum::abit_},
		{caoco::tk_enum::astr_}
		});
	auto exp_result2 = caoco::tokenizer(input_vec2.cbegin(), input_vec2.cend())();

	if (!exp_result2.valid()) {
		std::cout << exp_result2.error_message() << std::endl;
	}
	auto result2 = exp_result2.extract();
	for (auto& tk : result2) {
		std::cout << tk.type_to_string() << " ";
	}
	for (auto& tk : result2) {
		std::cout << tk.type_to_string() << " ";
	}
	std::cout << std::endl;
	for (size_t i = 0; i < result2.size(); ++i) {
		EXPECT_EQ(result2[i].type(), expected_result2[i].type());
		if (result2[i].type() != expected_result2[i].type()) {
			std::cout << "Tokenization result expected keyword token: " << expected_result2[i].type_to_string()
				<< " Got: " << result2[i].type_to_string() << std::endl;
		}
	}
}
#endif

#if CAOCO_TEST_TOKENIZER_KeywordsMixedShouldThrow
TEST(ut_Tokenizer_KeywordsMixedShouldThrow, ut_Tokenizer) {
	auto input_vec = caoco::sl::to_u8vec(u8"#include #macro #enter #start #type #var #class #obj #private\
 public #func #const #static #if #else #elif #while #for #on #break #continue #return #print #none #int #uint #real\
 #byte #bit #str");
	auto result = caoco::tokenizer(input_vec.cbegin(), input_vec.cend())();
	EXPECT_FALSE(result.valid());
	if(!result.valid()){
		std::cout << result.error_message() << std::endl;
	}


	auto input_vec2 = caoco::sl::to_u8vec(u8"include macro enter start type var class obj private\
 #public func const static if else elif while for on break continue return print none int uint real\
 byte bit str\0");

	result = caoco::tokenizer(input_vec2.cbegin(), input_vec2.cend())();
	EXPECT_FALSE(result.valid());
	if (!result.valid()) {
		std::cout << result.error_message() << std::endl;
	}
}
#endif

#if CAOCO_TEST_TOKENIZER_KeywordsDirectiveReportEarlyMisspell 
TEST(ut_Tokenizer_KeywordsDirectiveReportEarlyMisspell, ut_Tokenizer) {
	auto input_vec = caoco::sl::to_u8vec(u8"#inclde");
	auto result = caoco::tokenizer(input_vec.cbegin(), input_vec.cend())();
	EXPECT_FALSE(result.valid());
	if (!result.valid()) {
		std::cout << result.error_message() << std::endl;
	}
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Parser Basic Tests
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#if CAOCO_TEST_PARSER_BASIC
#define CAOCO_TEST_PARSER_SingleNodes 1
#define CAOCO_TEST_PARSER_ValueExpressions 1
#define CAOCO_TEST_PARSER_ValueStatementREPL 0
#define CAOCO_TEST_PARSER_ValueStatements 1
#endif

#if CAOCO_TEST_PARSER_SingleNodes
TEST(ut_Parser_Operands, ut_Parser) {
	// Operands.
	caoco::astnode expected_result[] = {
		caoco::astnode(caoco::astnode_enum::number_literal_, u8"1"),
		caoco::astnode(caoco::astnode_enum::real_literal_, u8"1.1"),
		caoco::astnode(caoco::astnode_enum::string_literal_, u8"'string literal'"),
		caoco::astnode(caoco::astnode_enum::string_literal_, u8"'\\\''"),
		caoco::astnode(caoco::astnode_enum::alnumus_, u8"alnumus"),
		caoco::astnode(caoco::astnode_enum::byte_literal_, u8"1c"),
		caoco::astnode(caoco::astnode_enum::bit_literal_, u8"1b"),
		caoco::astnode(caoco::astnode_enum::unsigned_literal_, u8"1u"),
		caoco::astnode(caoco::astnode_enum::none_literal_, u8"#none")
	};

	// Number literals.
	EXPECT_TRUE(test_and_compare_parsing_function_from_u8(
		"Operands",
		&caoco::parse_operand,
		expected_result[0],
		u8"1\0"
	));

	// Real literals
	EXPECT_TRUE(test_and_compare_parsing_function_from_u8(
		"Operands",
		&caoco::parse_operand,
		expected_result[1],
		u8"1.1\0"
	));

	// String literals
	EXPECT_TRUE(test_and_compare_parsing_function_from_u8(
		"Operands",
		&caoco::parse_operand,
		expected_result[2],
		u8"'string literal'\0"
	));

	// String literals with escape
	EXPECT_TRUE(test_and_compare_parsing_function_from_u8(
		"Operands",
		&caoco::parse_operand,
		expected_result[3],
		u8"'\\\''\0"
	));

	// Alnumus
	EXPECT_TRUE(test_and_compare_parsing_function_from_u8(
		"Operands",
		&caoco::parse_operand,
		expected_result[4],
		u8"alnumus\0"
	));

	// Byte literals
	EXPECT_TRUE(test_and_compare_parsing_function_from_u8(
		"Operands",
		&caoco::parse_operand,
		expected_result[5],
		u8"1c\0"
	));

	// Bit literals
	EXPECT_TRUE(test_and_compare_parsing_function_from_u8(
		"Operands",
		&caoco::parse_operand,
		expected_result[6],
		u8"1b\0"
	));

	// Unsigned literals
	EXPECT_TRUE(test_and_compare_parsing_function_from_u8(
		"Operands",
		&caoco::parse_operand,
		expected_result[7],
		u8"1u\0"
	));

	// None literals
	EXPECT_TRUE(test_and_compare_parsing_function_from_u8(
		"Operands",
		&caoco::parse_operand,
		expected_result[8],
		u8"#none\0"
	));
}

TEST(ut_Parser_Arguments, ut_Parser) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_parsing_function_from_u8(
		"Arguments",
		&caoco::parse_arguments,
		caoco::astnode(caoco::astnode_enum::arguments_, u8"()"),
		u8"()\0"
	));

	EXPECT_TRUE(test_and_compare_parsing_function_from_u8(
		"Arguments",
		&caoco::parse_arguments,
		astnode(astnode_enum::arguments_, u8"(1)", astnode(astnode_enum::number_literal_, u8"1")),
		u8"(1)\0"
	));


	EXPECT_TRUE(test_and_compare_parsing_function_from_u8(
		"Arguments",
		&caoco::parse_arguments,
		astnode(astnode_enum::arguments_, u8"(1,1,3)",
			{ astnode(astnode_enum::number_literal_, u8"1"),
			astnode(astnode_enum::number_literal_, u8"1"),
			astnode(astnode_enum::number_literal_, u8"3") }),
		u8"(1, 1, 3)\0"
	));
}
#endif

#if CAOCO_TEST_PARSER_ValueExpressions
TEST(ut_Parser_Expression_SingleOperand, LiteralInt) {
	using namespace caoco;

	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"SingleOperand_LiteralInt",
		astnode(astnode_enum::number_literal_, u8"1"),
		u8"1\0"
	));
}

TEST(ut_Parser_Expression_SingleOperand, LiteralReal) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"SingleOperand_LiteralReal",
		astnode(astnode_enum::real_literal_, u8"1.1"),
		u8"1.1\0"
	));
}

TEST(ut_Parser_Expression_SingleOperand, LiteralUint) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"SingleOperand_LiteralUint",
		astnode(astnode_enum::unsigned_literal_, u8"1u"),
		u8"1u\0"
	));
}

TEST(ut_Parser_Expression_SingleOperand, LiteralBit) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"SingleOperand_LiteralBit",
		astnode(astnode_enum::bit_literal_, u8"1b"),
		u8"1b\0"
	));
}

TEST(ut_Parser_Expression_SingleOperand, LiteralString) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"SingleOperand_LiteralString",
		astnode(astnode_enum::string_literal_, u8"'hello'"),
		u8"'hello'\0"
	));
}

TEST(ut_Parser_Expression_SingleOperand, LiteralByte) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"SingleOperand_LiteralByte",
		astnode(astnode_enum::byte_literal_, u8"1c"),
		u8"1c\0"
	));
}

TEST(ut_Parser_Expression_SingleOperand, LiteralNone) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"SingleOperand_LiteralNone",
		astnode(astnode_enum::none_literal_, u8"#none"),
		u8"#none\0"
	));
}

TEST(ut_Parser_Expression_SingleOperand, ValueInBrackets) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"SingleOperand_ValueInBrackets",
		astnode(astnode_enum::number_literal_, u8"1"),
		u8"(1)\0"
	));
}

TEST(ut_Parser_Expression_SingleOperation, Binary) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"SingleOperation_BinaryAddition",
		astnode(astnode_enum::addition_, u8"+",
			astnode(astnode_enum::number_literal_, u8"1"),
			astnode(astnode_enum::number_literal_, u8"1")
		),
		u8"1 + 1\0"
	));
}

TEST(ut_Parser_Expression_SingleOperation, EmptyScopeIsAnError) {
	auto input_vec = caoco::sl::to_u8vec(u8"()\0");
	auto result = caoco::tokenizer(input_vec.cbegin(), input_vec.cend())();
	auto parse_result = caoco::expression_split_parse(caoco::tk_cursor(result.expected().cbegin(), result.expected().cend()));
	EXPECT_FALSE(parse_result.valid());
}

TEST(ut_Parser_Expression_SingleOperation, FunctionCall) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"SingleOperation_FunctionCall",
		astnode(astnode_enum::function_call_, u8"()",
			{ astnode(astnode_enum::alnumus_, u8"foo"),
			astnode(astnode_enum::arguments_,u8"()") }),
		u8"foo()\0"
	));
}

TEST(ut_Parser_Expression_SingleOperation, Unary) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"SingleOperation_UnaryNegation",
		astnode(astnode_enum::negation_, u8"!",
			{ astnode(astnode_enum::number_literal_, u8"1") }),
		u8"!1\0"
	));
}

TEST(ut_Parser_Expression_ChainOperation, BinaryDiffPriority) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"ChainOperation_BinaryDiffPriority",
		astnode(astnode_enum::addition_, u8"+",
			{ astnode(astnode_enum::number_literal_, u8"1"),
			astnode(astnode_enum::multiplication_, u8"*",
				{ astnode(astnode_enum::number_literal_, u8"1"),
				astnode(astnode_enum::number_literal_, u8"1") }) }),
		u8"1 + 1 * 1\0"
	));
}

TEST(ut_Parser_Expression_ChainOperation,LogicalOperators) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"ChainOperation_LogicalOperators",
		astnode(astnode_enum::logical_AND_, u8"&&",
			{ astnode(astnode_enum::logical_OR_, u8"||",
				{ astnode(astnode_enum::alnumus_, u8"a"),
				astnode(astnode_enum::alnumus_, u8"b") }),
			astnode(astnode_enum::alnumus_, u8"c") }),
		u8"a || b && c\0"
	));
}

TEST(ut_Parser_Expression_ChainOperation,Scopes) {
	//[multiplication_] * |
	//	[addition_] + |
	//	[number_literal_] 1 |
	//	[number_literal_] 1 |
	//	[number_literal_] 1 |
	using namespace caoco;
	auto expected = astnode(astnode_enum::multiplication_, u8"*",
		{ astnode(astnode_enum::addition_, u8"+",
			{ astnode(astnode_enum::number_literal_, u8"1"),
			astnode(astnode_enum::number_literal_, u8"1") }),
		astnode(astnode_enum::number_literal_, u8"1") });
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"ChainOperation_Scopes",
		expected,
		u8"(1 + 1) * 1\0"
	));
}

TEST(ut_Parser_Expression_ChainOperation,AssingmentIsRightAssoc) {
	//a = b = c is parsed as a = (b = c), and not as (a = b) = c because of right-to-left associativity of assignment
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"ChainOperation_AssingmentIsRightAssoc",
		astnode(astnode_enum::simple_assignment_, u8"=",
			{ astnode(astnode_enum::alnumus_, u8"a"),
			astnode(astnode_enum::simple_assignment_, u8"=",
				{ astnode(astnode_enum::alnumus_, u8"b"),
				astnode(astnode_enum::alnumus_, u8"c") }) }),
		u8"a = b = c\0"
	));
}

TEST(ut_Parser_Expression_ChainOperation,SumIsLeftAssoc) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"ChainOperation_SumIsLeftAssoc",
		astnode(astnode_enum::subtraction_, u8"-",
			{ astnode(astnode_enum::addition_, u8"+",
				{ astnode(astnode_enum::alnumus_, u8"a"),
				astnode(astnode_enum::alnumus_, u8"b") }),
			astnode(astnode_enum::alnumus_, u8"c") }),
		u8"a + b - c\0"
	));
}

TEST(ut_Parser_Expression_ChainOperation,MemberAccessIsLeftAssoc) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"ChainOperation_MemberAccessIsLeftAssoc",
		astnode(astnode_enum::period_, u8".",
			{ astnode(astnode_enum::period_, u8".",
				{ astnode(astnode_enum::alnumus_, u8"a"),
				astnode(astnode_enum::alnumus_, u8"b") }),
			astnode(astnode_enum::alnumus_, u8"c") }),
		u8"a.b.c\0"
	));
}

TEST(ut_Parser_Expression_ChainOperation,BinaryAfterUnaryIsAnError) {

	auto input_vec = caoco::sl::to_u8vec(u8"!+1\0");
	auto result = caoco::tokenizer(input_vec.cbegin(), input_vec.cend())();
	auto parse_result = caoco::expression_split_parse(caoco::tk_cursor(result.expected().cbegin(), result.expected().cend()));

	EXPECT_FALSE(parse_result.valid());
}

TEST(ut_Parser_Expression_ChainOperation,UnaryRepeated) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"ChainOperation_UnaryRepeated",
		astnode(astnode_enum::negation_, u8"!",
			{ astnode(astnode_enum::negation_, u8"!",
				{ astnode(astnode_enum::number_literal_, u8"1") }) }),
		u8"!!1\0"
	));
}

TEST(ut_Parser_Expression_ChainOperation,UnaryThenBinary) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"ChainOperation_UnaryThenBinary",
		astnode(astnode_enum::addition_, u8"+",
			{ astnode(astnode_enum::negation_, u8"!",
				{ astnode(astnode_enum::number_literal_, u8"1") }),
			astnode(astnode_enum::number_literal_, u8"1") }),
		u8"!1 + 1\0"
	));
}

TEST(ut_Parser_Expression_ChainOperation,UnaryThenHigherPriority) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"ChainOperation_UnaryThenHigherPriority",
		astnode(astnode_enum::multiplication_, u8"*",
			{ astnode(astnode_enum::negation_, u8"!",
				{ astnode(astnode_enum::number_literal_, u8"1") }),
			astnode(astnode_enum::number_literal_, u8"1") }),
		u8"!1 * 1\0"
	));
}

TEST(ut_Parser_Expression_ChainOperation,UnaryAfterBinary) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"ChainOperation_UnaryAfterBinary",
		astnode(astnode_enum::addition_, u8"+",
			{ astnode(astnode_enum::number_literal_, u8"1"),
			astnode(astnode_enum::negation_, u8"!",
				{ astnode(astnode_enum::number_literal_, u8"1") }) }),
		u8"1 + !1\0"
	));
}

TEST(ut_Parser_Expression_ChainOperation,UnaryThenFunctionCall) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"ChainOperation_UnaryThenFunctionCall",
		astnode(astnode_enum::negation_, u8"!",
			{ astnode(astnode_enum::function_call_, u8"()",
				{ astnode(astnode_enum::alnumus_, u8"foo"),
				astnode(astnode_enum::arguments_, u8"()") }) }),
		u8"!foo()\0"
	));
}

TEST(ut_Parser_Expression_ChainOperation,FunctionCallThenBinary) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"ChainOperation_FunctionCallThenBinary",
		astnode(astnode_enum::addition_, u8"+",
			{ astnode(astnode_enum::function_call_, u8"()",
				{ astnode(astnode_enum::alnumus_, u8"foo"),
				astnode(astnode_enum::arguments_, u8"()") }),
			astnode(astnode_enum::number_literal_, u8"1") }),
		u8"foo() + 1\0"
	));
}

TEST(ut_Parser_Expression_ChainOperation,BinaryDotOperatorThenFunctionCall) {
	using namespace caoco;
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"ChainOperation_BinaryDotOperatorThenFunctionCall",
		astnode(astnode_enum::function_call_, u8"()",
			{ astnode(astnode_enum::period_, u8".",
				{ astnode(astnode_enum::alnumus_, u8"foo"),
				astnode(astnode_enum::alnumus_, u8"bar") }),
			astnode(astnode_enum::arguments_, u8"()") }),
		u8"foo.bar()\0"
	));
}

TEST(ut_Parser_Expression_ChainOperation,BinaryThenFunctionCall) {
	//[addition_] + |
	//	[number_literal_] 1 |
	//	[function_call_]() |
	//		[alnumus_] foo |
	//		[arguments_] |
	using namespace caoco;
	auto expected = astnode(astnode_enum::addition_, u8"+",
		{ astnode(astnode_enum::number_literal_, u8"1"),
		astnode(astnode_enum::function_call_, u8"()",
			{ astnode(astnode_enum::alnumus_, u8"foo"),
			astnode(astnode_enum::arguments_, u8"()") }) });
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"ChainOperation_BinaryThenFunctionCall",
		expected,
		u8"1 + foo()\0"
	));
}

TEST(ut_Parser_Expression_ChainOperation,MemberAccessWithFunctionCall) {
	// a.b().c is parsed as (a.b()).c and not as a.(b().c) because of left-to-right associativity of member access.
	using namespace caoco;
	// create ast for a.b().c
	//[period_] .|
	//[function_call_] () |
	//	[period_] . |
	//	[alnumus_] a |
	//	[alnumus_] b |
	//	[arguments_]().c |
	//	[alnumus_] c |
	astnode expected = astnode(astnode_enum::period_, u8".",
		{ astnode(astnode_enum::function_call_, u8"()",
			{ astnode(astnode_enum::period_, u8".",
				{ astnode(astnode_enum::alnumus_, u8"a"),
				astnode(astnode_enum::alnumus_, u8"b") }),
			astnode(astnode_enum::arguments_, u8"()") }),
		astnode(astnode_enum::alnumus_, u8"c") });
	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"ChainOperation_MemberAccessWithFunctionCall",
		expected,
		u8"a.b().c\0"
	));
}

TEST(ut_Parser_Expression_ComplexOperation, Operation) {
	//addition_] + |
	//[function_call_]() |
	//	  [period_] . |
	//		[alnumus_] foo |
	//		[alnumus_] bar |
	//	  [arguments_]() |
	//	[multiplication_] * |
	//	  [number_literal_] 1 |
	//	  [number_literal_] 1 |
	using namespace caoco;
	auto expected = astnode(astnode_enum::addition_, u8"+",
		{ astnode(astnode_enum::function_call_, u8"()",
			{ astnode(astnode_enum::period_, u8".",
				{ astnode(astnode_enum::alnumus_, u8"foo"),
				astnode(astnode_enum::alnumus_, u8"bar") }),
			astnode(astnode_enum::arguments_, u8"()") }),
		astnode(astnode_enum::multiplication_, u8"*",
			{ astnode(astnode_enum::number_literal_, u8"1"),
			astnode(astnode_enum::number_literal_, u8"1") }) });

	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"ComplexOperation",
		expected,
		u8"foo.bar() + 1 * 1\0"
	));
}

TEST(ut_Parser_Expression_ComplexOperation, OperationWithScopes) {
	//[multiplication_] * |
	//	[addition_] + |
	//		[function_call_]() |
	//			[period_] . |
	//				[alnumus_] foo |
	//				[alnumus_] bar |
	//			[arguments_]() |
	//		[number_literal_] 1 |
	//	[number_literal_] 1 |
	using namespace caoco;
	auto expected = astnode(astnode_enum::multiplication_, u8"*",
		{ astnode(astnode_enum::addition_, u8"+",
			{ astnode(astnode_enum::function_call_, u8"()",
				{ astnode(astnode_enum::period_, u8".",
					{ astnode(astnode_enum::alnumus_, u8"foo"),
					astnode(astnode_enum::alnumus_, u8"bar") }),
				astnode(astnode_enum::arguments_, u8"()") }),
			astnode(astnode_enum::number_literal_, u8"1") }),
		astnode(astnode_enum::number_literal_, u8"1") });


	EXPECT_TRUE(test_and_compare_split_parsing_function_from_u8(
		"ComplexOperationWithScopes",
		expected,
		u8"(foo.bar() + 1) * 1\0"
	));
}
#endif

#if CAOCO_TEST_PARSER_ValueStatementREPL
TEST(ut_repl, ut_Parser) {
	while (true) {
		std::cout << "Caoco Expression AST REPL" << std::endl;
		std::string input;
		std::cout << "Please enter an value statement or #exit to exit: ";
		std::getline(std::cin, input);
		if (input == "#exit") break;
		else {
			auto input_vec = caoco::sl::to_u8vec(input.c_str());
			auto result = caoco::tokenizer(input_vec.cbegin(), input_vec.cend())();
			if(!result.valid()){
				std::cout << result.error_message() << std::endl;
			}
			else {
				auto exp_result = result.expected();
				auto parse_result = caoco::parse_value_statement(exp_result.cbegin(), exp_result.cend());
				if (!parse_result.valid()) std::cout << parse_result.error_message() << std::endl;
				else print_ast(parse_result.expected());
			}
		}
	}
}
#endif

#if CAOCO_TEST_PARSER_ValueStatements
TEST(ut_Parser_ValueStatements, ut_Parser) {

	EXPECT_TRUE(test_and_compare_parsing_function_from_u8(
		"PrimaryExpr",
		&caoco::parse_value_statement,
		caoco::astnode(caoco::astnode_enum::alnumus_, u8"foo"),
		u8"foo;"
	));

	// foo + 2
	EXPECT_TRUE(test_and_compare_parsing_function_from_u8(
		"BinaryExpr",
		&caoco::parse_value_statement,
		caoco::astnode(caoco::astnode_enum::addition_, u8"+",
			{ caoco::astnode(caoco::astnode_enum::alnumus_, u8"foo"),
			caoco::astnode(caoco::astnode_enum::number_literal_, u8"2") }),
		u8"foo + 2;"
	));

	// foo = 1 + 2;
	EXPECT_TRUE(test_and_compare_parsing_function_from_u8(
		"AssignmentExpr",
		&caoco::parse_value_statement,
		caoco::astnode(caoco::astnode_enum::simple_assignment_, u8"=",
			{ caoco::astnode(caoco::astnode_enum::alnumus_, u8"foo"),
			caoco::astnode(caoco::astnode_enum::addition_, u8"+",
				{ caoco::astnode(caoco::astnode_enum::number_literal_, u8"1"),
				caoco::astnode(caoco::astnode_enum::number_literal_, u8"2") }) }),
		u8"foo = 1 + 2;"
	));

	// Testing period(member access) operator
	EXPECT_TRUE(test_and_compare_parsing_function_from_u8(
		"PeriodOperator",
		&caoco::parse_value_statement,
		caoco::astnode(caoco::astnode_enum::period_, u8".",
			{ caoco::astnode(caoco::astnode_enum::alnumus_, u8"foo"),
			caoco::astnode(caoco::astnode_enum::alnumus_, u8"bar") }),
		u8"foo.bar;"
	));

	// Testing function call operator ()
	EXPECT_TRUE(test_and_compare_parsing_function_from_u8(
		"FunctionCallOperator",
		&caoco::parse_value_statement,
		caoco::astnode(caoco::astnode_enum::function_call_, u8"()",
			{ caoco::astnode(caoco::astnode_enum::alnumus_, u8"foo"),
			caoco::astnode(caoco::astnode_enum::arguments_, u8"()") }),
		u8"foo();"
	));

	// Statement with no following binary operator should be invalid and throw an exception.
	std::cout << "Testing foo=; Result should be invalid." << std::endl;
	EXPECT_FALSE(test_and_compare_parsing_function_from_u8(
	"InvalidStatement",
	&caoco::parse_value_statement,
	caoco::astnode(caoco::astnode_enum::alnumus_, u8"foo"),
	u8"foo=;"
	));

}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Parser Utils Tests
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#if CAOCO_TEST_PARSER_UTILS
#define CAOCO_TEST_PARSER_UTILS_BasicScopeFinder 1
#define CAOCO_TEST_PARSER_UTILS_ListScopeFinder 1
#define CAOCO_TEST_PARSER_UTILS_FrameScopeFinder 1
#define CAOCO_TEST_PARSER_UTILS_StatementScopeFinder 1
#endif
#if CAOCO_TEST_PARSER_UTILS_BasicScopeFinder 
TEST(CaocoParser_BasicNode_BasicScopes, CaocoParser_Test) {
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_parser_scopes.candi");
	auto result = caoco::tokenizer(source_file.cbegin(), source_file.cend())().expected();

	// empty scope
	std::cout << "Testing empty scope:" << std::endl;
	caoco::parser_scope_result empty_scope = caoco::find_paren_scope(result.cbegin(), result.cend());
	EXPECT_TRUE(empty_scope.valid);
	for (auto i = empty_scope.scope_begin(); i != empty_scope.scope_end(); i++) std::cout << i->literal_str();
	std::cout << std::endl;

	// scope with 1 element
	std::cout << "Testing scope with 1 element:" << std::endl;
	caoco::parser_scope_result scope_with_1_element = caoco::find_paren_scope(empty_scope.scope_end(), result.cend());
	EXPECT_TRUE(scope_with_1_element.valid);
	for(auto i = scope_with_1_element.scope_begin(); i != scope_with_1_element.scope_end(); i++) std::cout << i->literal_str();
	std::cout << std::endl;
	// double scope
	std::cout << "Testing double scope:" << std::endl;
	caoco::parser_scope_result double_scope = caoco::find_paren_scope(scope_with_1_element.scope_end(), result.cend());
	EXPECT_TRUE(double_scope.valid);
	for (auto i = double_scope.scope_begin(); i != double_scope.scope_end(); i++) std::cout << i->literal_str();
	std::cout << std::endl;
	// complex scope
	std::cout << "Testing complex scope:" << std::endl;
	caoco::parser_scope_result complex_scope = caoco::find_paren_scope(double_scope.scope_end(), result.cend());
	EXPECT_TRUE(complex_scope.valid);
	for (auto i = complex_scope.scope_begin(); i != complex_scope.scope_end(); i++) std::cout << i->literal_str();
	std::cout << std::endl;
	// complex scope with lists
	std::cout << "Testing complex scope with lists:" << std::endl;
	caoco::parser_scope_result complex_scope_with_lists = caoco::find_paren_scope(complex_scope.scope_end(), result.cend());
	EXPECT_TRUE(complex_scope_with_lists.valid);
	for (auto i = complex_scope_with_lists.scope_begin(); i != complex_scope_with_lists.scope_end(); i++) std::cout << i->literal_str();
	std::cout << std::endl;
	// complex scope with frames and lists
	std::cout << "Testing complex scope with frames and lists:" << std::endl;
	caoco::parser_scope_result complex_scope_with_frames_and_lists = caoco::find_paren_scope(complex_scope_with_lists.scope_end(), result.cend());
	EXPECT_TRUE(complex_scope_with_frames_and_lists.valid);


	// Invalid scope should be invalid
	auto source_file2 = caoco::sl::to_char8_vector("(()");
	auto result2 = caoco::tokenizer(source_file2.cbegin(), source_file2.cend())().expected();

	std::cout << "Testing invalid scope:";
	caoco::parser_scope_result invalid_list = caoco::find_paren_scope(result2.cbegin(), result2.cend());
	EXPECT_FALSE(invalid_list.valid);
	std::cout << "scope error message:" << invalid_list.error_message << std::endl;
}
#endif

#if CAOCO_TEST_PARSER_UTILS_ListScopeFinder 
TEST(ut_PARSER_UTILS, ListScopeFinder) {
	auto source_file = caoco::sl::to_char8_vector("{}{a}{{}}{({})[{}]{}}{{}");
	auto result = caoco::tokenizer(source_file.cbegin(), source_file.cend())().expected();

	// empty list
	std::cout << "Testing empty list:";
	caoco::parser_scope_result empty_list = caoco::find_list_scope(result.cbegin(), result.cend());
	EXPECT_TRUE(empty_list.valid);
	for (auto i = empty_list.scope_begin(); i != empty_list.scope_end(); i++) std::cout << i->literal_str();
	std::cout << std::endl;

	// list with 1 element
	std::cout << "Testing list with 1 element:";
	caoco::parser_scope_result list_with_1_element = caoco::find_list_scope(empty_list.scope_end(), result.cend());
	EXPECT_TRUE(list_with_1_element.valid);
	for (auto i = list_with_1_element.scope_begin(); i != list_with_1_element.scope_end(); i++) std::cout << i->literal_str();
	std::cout << std::endl;

	// double list
	std::cout << "Testing double list:";
	caoco::parser_scope_result double_list = caoco::find_list_scope(list_with_1_element.scope_end(), result.cend());
	EXPECT_TRUE(double_list.valid);
	for (auto i = double_list.scope_begin(); i != double_list.scope_end(); i++) std::cout << i->literal_str();
	std::cout << std::endl;

	// complex list
	std::cout << "Testing complex list:";
	caoco::parser_scope_result complex_list = caoco::find_list_scope(double_list.scope_end(), result.cend());
	EXPECT_TRUE(complex_list.valid);
	for (auto i = complex_list.scope_begin(); i != complex_list.scope_end(); i++) std::cout << i->literal_str();
	std::cout << std::endl;
	
	std::cout << "Testing invalid list:";
	caoco::parser_scope_result invalid_list = caoco::find_list_scope(complex_list.scope_end(), result.cend());
	EXPECT_FALSE(invalid_list.valid);
	std::cout << "scope error message:" << invalid_list.error_message << std::endl;

}
#endif

#if CAOCO_TEST_PARSER_UTILS_FrameScopeFinder 
TEST(ut_PARSER_UTILS, FrameScopeFinder) {
	auto source_file = caoco::sl::to_char8_vector("[][a][[]][([])[[]][]][[]");
	auto result = caoco::tokenizer(source_file.cbegin(), source_file.cend())().expected();

	// empty frame
	std::cout << "Testing empty frame:";
	caoco::parser_scope_result empty_frame = caoco::find_frame_scope(result.cbegin(), result.cend());
	EXPECT_TRUE(empty_frame.valid);
	for (auto i = empty_frame.scope_begin(); i != empty_frame.scope_end(); i++) std::cout << i->literal_str();
	std::cout << std::endl;

	// frame with 1 element
	std::cout << "Testing frame with 1 element:";
	caoco::parser_scope_result frame_with_1_element = caoco::find_frame_scope(empty_frame.scope_end(), result.cend());
	EXPECT_TRUE(frame_with_1_element.valid);
	for (auto i = frame_with_1_element.scope_begin(); i != frame_with_1_element.scope_end(); i++) std::cout << i->literal_str();
	std::cout << std::endl;

	// double frame
	std::cout << "Testing double frame:";
	caoco::parser_scope_result double_frame = caoco::find_frame_scope(frame_with_1_element.scope_end(), result.cend());
	EXPECT_TRUE(double_frame.valid);
	for (auto i = double_frame.scope_begin(); i != double_frame.scope_end(); i++) std::cout << i->literal_str();
	std::cout << std::endl;

	// complex frame
	std::cout << "Testing complex frame:";
	caoco::parser_scope_result complex_frame = caoco::find_frame_scope(double_frame.scope_end(), result.cend());
	EXPECT_TRUE(complex_frame.valid);
	for (auto i = complex_frame.scope_begin(); i != complex_frame.scope_end(); i++) std::cout << i->literal_str();
	std::cout << std::endl;

	std::cout << "Testing invalid frame:";
	caoco::parser_scope_result invalid_frame = caoco::find_frame_scope(complex_frame.scope_end(), result.cend());
	EXPECT_FALSE(invalid_frame.valid);
	std::cout << "scope error message:" << invalid_frame.error_message << std::endl;

}
#endif

#if CAOCO_TEST_PARSER_UTILS_StatementScopeFinder
TEST(CaocoParser_BasicNode_StatementScope, CaocoParser_Test) {
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_parser_statementscope.candi");
	auto result = caoco::tokenizer(source_file.cbegin(), source_file.cend())().extract();

	// Single value statement : 1;
	std::cout << "Testing single value statement" << std::endl;
	caoco::parser_scope_result empty_statement = caoco::find_statement(caoco::tk_enum::number_literal_, caoco::tk_enum::eos_, result.cbegin(), result.cend());
	EXPECT_TRUE(empty_statement.valid);

	// statement with multiple tokens: #var a = 1;
	std::cout << "Testing statement with multiple tokens" << std::endl;
	caoco::parser_scope_result multiple_token_statement = caoco::find_statement(caoco::tk_enum::var_, caoco::tk_enum::eos_, empty_statement.scope_end(), result.cend());
	EXPECT_TRUE(multiple_token_statement.valid);

	// statement with multiple tokens and scopes: #var a = (1;2;3);
	std::cout << "Testing statement with multiple tokens and scopes" << std::endl;
	caoco::parser_scope_result multiple_token_scope_statement = caoco::find_statement(caoco::tk_enum::var_, caoco::tk_enum::eos_, multiple_token_statement.scope_end(), result.cend());
	EXPECT_TRUE(multiple_token_scope_statement.valid);

	// statement with lists frames and scopes nested in diffrent ways containing end tokens. #var a = 1 + ([ 2 ;3 + {4;5;6}]);
	std::cout << "Testing statement with lists frames and scopes nested in diffrent ways containing end tokens." << std::endl;
	caoco::parser_scope_result complex_statement = caoco::find_statement(caoco::tk_enum::var_, caoco::tk_enum::eos_, multiple_token_scope_statement.scope_end(), result.cend());
	EXPECT_TRUE(complex_statement.valid);
	//EXPECT_TRUE(complex_statement.scope_end() == result.cend() - 1);

	// Test finding an "open" statement which allows for repeated open tokens. ex a = a + a + ([ a ;a + {a;a;a}]);
	std::cout << "Testing statement with lists frames and scopes nested in diffrent ways containing begin and end tokens." << std::endl;
	caoco::parser_scope_result open_statement = caoco::find_open_statement(caoco::tk_enum::alnumus_, caoco::tk_enum::eos_, complex_statement.scope_end(), result.cend());
	EXPECT_TRUE(open_statement.valid);
	EXPECT_TRUE(open_statement.scope_end() == result.cend() - 1);

}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Parser Statement Tests
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#if CAOCO_TEST_PARSER_STATEMENTS
#define CAOCO_TEST_PARSER_STATEMENTS_TypeAlias 1
#define CAOCO_TEST_PARSER_STATEMENTS_VariableDeclaration 1
#define CAOCO_TEST_PARSER_STATEMENTS_Functions 1
#define CAOCO_TEST_PARSER_STATEMENTS_Classes 1
#define CAOCO_TEST_PARSER_STATEMENTS_Conditional 1
#define CAOCO_TEST_PARSER_STATEMENTS_SwitchStatement 1
#define CAOCO_TEST_PARSER_STATEMENTS_WhileLoop 1
#define CAOCO_TEST_PARSER_STATEMENTS_ForLoop 1
#define CAOCO_TEST_PARSER_STATEMENTS_ReturnStatement 1
#define CAOCO_TEST_PARSER_STATEMENTS_SeperatedList 1
#define CAOCO_TEST_PARSER_STATEMENTS_ListOperand 1
#define CAOCO_TEST_PARSER_STATEMENTS_PragmaticBlock 1
#define CAOCO_TEST_PARSER_STATEMENTS_FunctionalBlock 1
#endif

#if CAOCO_TEST_PARSER_STATEMENTS_TypeAlias
TEST(ut_Parser_TypeAlias, ut_Parser) {
	// type alias
	//use IntAlias = int;
	// type_alias expression
	//use Int = int(0 - 100);
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_parser_typealias.candi");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {

		auto result = exp_result.expected();

		std::cout << "Testing use IntAlias = int;" << std::endl;
		auto px2 = caoco::parse_directive_type(result.cbegin(), result.cend());
		EXPECT_TRUE(px2.valid());
		if (px2.valid()) print_ast(px2.expected());
		else std::cout << px2.error_message() << std::endl;

		std::cout << "Testing use Int = int(0 - 100);" << std::endl;
		auto px1 = caoco::parse_directive_type(px2.always(), result.cend());
		EXPECT_TRUE(px1.valid());
		if (px1.valid()) print_ast(px1.expected());
		else std::cout << px1.error_message() << std::endl;


	}
	else{
		std::cout << exp_result.error_message() << std::endl;
	}
}
#endif

#if CAOCO_TEST_PARSER_STATEMENTS_VariableDeclaration
TEST(ut_Parser_VariableDeclaration, ut_Parser) {
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_parser_variabledecl.candi");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();

		// Anon Var Decl
		std::cout << "Testing foo;" << std::endl;
		auto anon_var_decl = caoco::parse_directive_var(result.begin(), result.cend());
		EXPECT_TRUE(anon_var_decl.valid());
		if(anon_var_decl.valid()) print_ast(anon_var_decl.expected());
		else std::cout << anon_var_decl.error_message() << std::endl;

		// Anon Var Decl Assingment
		std::cout << "Testing foo = 1;" << std::endl;
		auto anon_var_decl_assign = caoco::parse_directive_var(anon_var_decl.always(), result.cend());
		EXPECT_TRUE(anon_var_decl_assign.valid());
		if(anon_var_decl_assign.valid()) print_ast(anon_var_decl_assign.expected());
		else std::cout << anon_var_decl_assign.error_message() << std::endl;	

		// Complex Anon Var Decl Assingment
		// #var foo = 1 + c * (3 / 4);
		std::cout << "Testing #var foo = 1 + c * (3 / 4);" << std::endl;
		auto anon_var_decl_assign_complex = caoco::parse_directive_var(anon_var_decl_assign.always(), result.cend());
		EXPECT_TRUE(anon_var_decl_assign_complex.valid());
		if(anon_var_decl_assign_complex.valid()) print_ast(anon_var_decl_assign_complex.expected());
		else std::cout << anon_var_decl_assign_complex.error_message() << std::endl;

		// Type Contrained Var Decl
		std::cout << "foo int;" << std::endl;
		auto type_constrained_var_decl = caoco::parse_directive_var(anon_var_decl_assign_complex.always(), result.cend());
		EXPECT_TRUE(type_constrained_var_decl.valid());
		if(type_constrained_var_decl.valid()) print_ast(type_constrained_var_decl.expected());
		else std::cout << type_constrained_var_decl.error_message() << std::endl;

		// Type Contrained Var Decl Assingment
		std::cout << "Testing foo Int = 1" << std::endl;
		auto type_constrained_var_decl_assign = caoco::parse_directive_var(type_constrained_var_decl.always(), result.cend());
		EXPECT_TRUE(type_constrained_var_decl_assign.valid());
		if(type_constrained_var_decl_assign.valid()) print_ast(type_constrained_var_decl_assign.expected());
		else std::cout << type_constrained_var_decl_assign.error_message() << std::endl;


		// Type Expr Contrained Var Decl
		std::cout << "Testing foo [int,Int];" << std::endl;
		auto type_constrained_var_decl_assign_complex = caoco::parse_directive_var(type_constrained_var_decl_assign.always(), result.cend());
		EXPECT_TRUE(type_constrained_var_decl_assign_complex.valid());
		if(type_constrained_var_decl_assign_complex.valid()) print_ast(type_constrained_var_decl_assign_complex.expected());
		else std::cout << type_constrained_var_decl_assign_complex.error_message() << std::endl;

		// Type Expr Contrained Var Decl Assingment
		std::cout << "Testing foo [int,Int] = 1" << std::endl;
		auto type_constrained_var_decl_assign_complex_assign = caoco::parse_directive_var(type_constrained_var_decl_assign_complex.always(), result.cend());
		EXPECT_TRUE(type_constrained_var_decl_assign_complex_assign.valid());
		if(type_constrained_var_decl_assign_complex_assign.valid()) print_ast(type_constrained_var_decl_assign_complex_assign.expected());
		else std::cout << type_constrained_var_decl_assign_complex_assign.error_message() << std::endl;

	}
	else {
		std::cout << exp_result.error_message() << std::endl;
	}
}
#endif

#if CAOCO_TEST_PARSER_STATEMENTS_Functions
TEST(ut_Parser_Functions, ut_Parser) {
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_parser_function.candi");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();

		auto shorthand_void_arg_method = caoco::parse_directive_func(result.cbegin(), result.cend());
		EXPECT_TRUE(shorthand_void_arg_method.valid());
		if (!shorthand_void_arg_method.valid()) {
			std::cout << shorthand_void_arg_method.error_message() << std::endl;
		}
		else
			print_ast(shorthand_void_arg_method.expected());

		auto unconstrained_method = caoco::parse_directive_func(shorthand_void_arg_method.always(), result.cend());
		EXPECT_TRUE(unconstrained_method.valid());
		if (!unconstrained_method.valid()) {
			std::cout << unconstrained_method.error_message() << std::endl;
		}
		else
			print_ast(unconstrained_method.expected());

		auto unconstrained_method_no_args = caoco::parse_directive_func(unconstrained_method.always(), result.cend());
		EXPECT_TRUE(unconstrained_method_no_args.valid());
		if (!unconstrained_method_no_args.valid()) {
			std::cout << unconstrained_method_no_args.error_message() << std::endl;
		}
		else
			print_ast(unconstrained_method_no_args.expected());

		auto constrained_shorthand_void_arg_method = caoco::parse_directive_func(unconstrained_method_no_args.always(), result.cend());
		EXPECT_TRUE(constrained_shorthand_void_arg_method.valid());
		if (!constrained_shorthand_void_arg_method.valid()) {
			std::cout << constrained_shorthand_void_arg_method.error_message() << std::endl;
		}
		else
		print_ast(constrained_shorthand_void_arg_method.expected());


		auto constrained_method = caoco::parse_directive_func(constrained_shorthand_void_arg_method.always(), result.cend());
		EXPECT_TRUE(constrained_method.valid());
		if (!constrained_method.valid()) {
			std::cout << constrained_method.error_message() << std::endl;
		}
		else
		print_ast(constrained_method.expected());
		
		auto function_with_return = caoco::parse_directive_func(constrained_method.always(), result.cend());
		EXPECT_TRUE(function_with_return.valid());
		if (!function_with_return.valid()) {
			std::cout << function_with_return.error_message() << std::endl;
		}
		else
			print_ast(function_with_return.expected());

		auto function_with_multiple_statements = caoco::parse_directive_func(function_with_return.always(), result.cend());
		EXPECT_TRUE(function_with_multiple_statements.valid());
		if (!function_with_multiple_statements.valid()) {
			std::cout << function_with_multiple_statements.error_message() << std::endl;
		}
		else
			print_ast(function_with_multiple_statements.expected());
	}
	else {
		std::cout << exp_result.error_message() << std::endl;
	}
}
#endif

#if CAOCO_TEST_PARSER_STATEMENTS_Classes
TEST(CaocoParser_BasicNode_Classes, CaocoParser_Test) {
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_parser_classes.candi");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();
		auto empty_class_def_end = test_parsing_function(
			"Empty Class Definition", &caoco::parse_directive_class, result.cbegin(), result.cend());
		auto class_def_with_members_end = test_parsing_function(
			"Class Definition with Members", &caoco::parse_directive_class, empty_class_def_end, result.cend());
		auto class_def_with_members_and_methods_end = test_parsing_function(
			"Class Definition with Members and Methods", &caoco::parse_directive_class, class_def_with_members_end, result.cend());
	}
	else {
		std::cout << exp_result.error_message() << std::endl;
	}
}
#endif

#if CAOCO_TEST_PARSER_STATEMENTS_Conditional
TEST(CaocoParser_ControlFlow, CaocoParser_Test) {
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_parser_conditional.candi");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();

		auto if_statement = test_parsing_function(
			"if statement", &caoco::parse_directive_if, result.cbegin(), result.cend());

		auto if_else_statement = test_parsing_function("if else statement", &caoco::parse_directive_if, if_statement, result.cend());

		auto if_elif_else = test_parsing_function("if elif else statement", &caoco::parse_directive_if, if_else_statement, result.cend());
	}
	else {
		std::cout << exp_result.error_message() << std::endl;
	}
}
#endif

#if CAOCO_TEST_PARSER_STATEMENTS_SwitchStatement
TEST(CaocoParser_SwitchStatement, CaocoParser_Test) {
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_parser_switch.candi");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();

		auto switch_statement = test_parsing_function(
			"switch statement", &caoco::parse_directive_on, result.cbegin(), result.cend());
	}
	else {
		std::cout << exp_result.error_message() << std::endl;
	}
}
#endif

#if CAOCO_TEST_PARSER_STATEMENTS_WhileLoop
TEST(CaocoParser_WhileLoop, CaocoParser_Test) {
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_parser_while.candi");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();

		auto while_loop = test_parsing_function(
			"while loop", &caoco::parse_directive_while, result.cbegin(), result.cend());
	}
	else {
		std::cout << exp_result.error_message() << std::endl;
	}
}
#endif

#if CAOCO_TEST_PARSER_STATEMENTS_ForLoop
TEST(CaocoParser_ForLoop, CaocoParser_Test) {
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_parser_for.candi");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();

		auto for_loop = test_parsing_function(
			"for loop", &caoco::parse_directive_for, result.cbegin(), result.cend());
	}
	else {
		std::cout << exp_result.error_message() << std::endl;
	}
}

#endif

#if CAOCO_TEST_PARSER_STATEMENTS_ReturnStatement 
TEST(CaocoParser_ReturnStatement, CaocoParser_Test) {
	auto source_file = caoco::sl::to_char8_vector("#return a;\0");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();

		auto return_statement = test_parsing_function(
			"return statement", &caoco::parse_directive_return, result.cbegin(), result.cend());
	}
	else {
		std::cout << exp_result.error_message() << std::endl;
	}
}
#endif

#if CAOCO_TEST_PARSER_STATEMENTS_SeperatedList 
TEST(CaocoParserUtils_SeperatedList, CaocoParser_Test) {
	auto source_file = caoco::sl::to_char8_vector("{(a),{b},[c],(a,b),{a,c},{a,d}}\0");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();

		auto seperated_list = caoco::find_seperated_list_scopes(result.cbegin(), result.cend(), caoco::tk_enum::comma_);

		int i = 0;
		for (auto& ascope : seperated_list) {
			std::cout << "Seperated List Arg" << i++ << ": ";
			for (auto a = ascope.scope_begin(); a != ascope.scope_end(); a++) {
				std::cout << a->literal_str();
			}
			std::cout << std::endl;
		}
	}
	else {
		std::cout << exp_result.error_message() << std::endl;
	}
}
#endif

#if CAOCO_TEST_PARSER_STATEMENTS_ListOperand
TEST(CaocoParserUtils_ListOperand, CaocoParser_Test) {
	auto source_file = caoco::sl::to_char8_vector("{(a),{b},c}\0");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();

		test_parsing_function(
			"List Operand", &caoco::parse_operand, result.cbegin(), result.cend());
	}
	else {
		std::cout << exp_result.error_message() << std::endl;
	}
}

TEST(CaocoParserUtils_ListOperand2, CaocoParser_Test) {
	auto source_file = caoco::sl::to_char8_vector("{a={wolf,dog};}\0");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();

		test_parsing_function(
			"List Operand", &caoco::parse_pragmatic_block, result.cbegin(), result.cend());
	}
	else {
		std::cout << exp_result.error_message() << std::endl;
	}
}

TEST(CaocoParserUtils_ListOperand3, CaocoParser_Test) {
	auto source_file = caoco::sl::to_char8_vector("{a={};}\0");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();

		test_parsing_function(
			"List Operand", &caoco::parse_pragmatic_block, result.cbegin(), result.cend());
	}
	else {
		std::cout << exp_result.error_message() << std::endl;
	}
}
#endif

#if CAOCO_TEST_PARSER_STATEMENTS_ListOperand

#endif

#if CAOCO_TEST_PARSER_STATEMENTS_PragmaticBlock
TEST(CaocoParser_BasicNode_PragmaticBlock, CaocoParser_Test) {
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_parser_pragmaticblock.candi");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();

		auto pragmatic_block_with_directives_end = test_parsing_function(
			"Pragmatic Block with Directives", &caoco::parse_pragmatic_block, result.cbegin(), result.cend());
	}
	else {
		std::cout << exp_result.error_message() << std::endl;
	}
}
#endif

#if CAOCO_TEST_PARSER_STATEMENTS_FunctionalBlock
TEST(CaocoParser_BasicNode_FunctionalBlock, CaocoParser_Test) {
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_parser_functionalblock.candi");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();

		auto functional_block_with_statements_end = test_parsing_function(
			"Functional Block with Statements", &caoco::parse_functional_block, result.cbegin(), result.cend());
	}
	else {
		std::cout << exp_result.error_message() << std::endl;
	}
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Parser Program Tests
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#if CAOCO_TEST_PARSER_PROGRAM
#define CAOCO_TEST_PARSER_PROGRAM_MinimumProgram 1
#define CAOCO_TEST_PARSER_PROGRAM_BasicProgram 1
#endif

#if CAOCO_TEST_PARSER_PROGRAM_MinimumProgram 
TEST(CaocoParser_MinimumProgram, CaocoParser_Test) {
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_parser_minimum_program.candi");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();

		try {
			auto parse_result = caoco::parse_program(result.cbegin(), result.cend());
			print_ast(parse_result);
		}
		catch (std::exception e) {
			std::cout << e.what() << std::endl;
		}
	}
	else {
		std::cout << exp_result.error_message() << std::endl;
	}
}
#endif


#if CAOCO_TEST_PARSER_PROGRAM_BasicProgram
TEST(CaocoParser_BasicProgram, CaocoParser_Test) {

	auto source_file = caoco::sl::load_file_to_char8_vector("ut_program_basic.candi");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();

		try {
			auto parse_result = caoco::parse_program(result.cbegin(), result.cend());
			print_ast(parse_result);
		}
		catch (std::exception e) {
			std::cout << e.what() << std::endl;
		}
	}
	else {
		std::cout << exp_result.error_message() << std::endl;
	}
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Preprocessor Tests
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#if CAOCO_TEST_PREPROCESSOR
#define CAOCO_TEST_PREPROCESSOR_Include  1
#define CAOCO_TEST_PREPROCESSOR_Macro 1
#endif

#if CAOCO_TEST_PREPROCESSOR_Include 
TEST(CaocoPreprocessor_Include, CaocoPreprocessor_Test) {
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_preprocessor_include.candi");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();
		auto preprocessed = caoco::preprocess(result, "ut_preprocessor_include.candi");

		if (!std::get<1>(preprocessed)) {
			std::cout << "Preprocessing Error: " << std::get<2>(preprocessed) << std::endl;
		}
		else {
			for (auto& token : std::get<0>(preprocessed)) {
				std::cout << token.literal_str();
			}
		}
	}
	else {
		std::cout << exp_result.error_message() << std::endl;
	}
}
#endif

#if CAOCO_TEST_PREPROCESSOR_Macro
TEST(CaocoPreprocessor_Macro, CaocoPreprocessor_Test) {
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_preprocessor_macros.candi");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();
		auto expanded = caoco::macro_expand(result, "ut_preprocessor_macros.candi");

		if(!std::get<1>(expanded)) {
			std::cout << "Macro Expansion Error: " << std::get<2>(expanded) << std::endl;
		}
		else {
			for (auto& token : std::get<0>(expanded)) {
				std::cout << token.literal_str();
			}
		}

	}
	else {
		std::cout << exp_result.error_message() << std::endl;
	}
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Const Evaluator Tests
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#if CAOCO_TEST_CONST_EVALUATOR
#define CAOCO_TEST_CONST_EVALUATOR_Literals 1
#define CAOCO_TEST_CONST_EVALUATOR_MathOperators 1
#define CAOCO_TEST_CONST_EVALUATOR_VariableDeclaration 1
#define CAOCO_TEST_CONST_EVALUATOR_Structs 1
#define CAOCO_TEST_CONST_EVALUATOR_FreeFunctions 1
#endif

#if CAOCO_TEST_CONST_EVALUATOR_Literals
TEST(CaocoConstantEvaluator_Literals, CaocoConstantEvaluator_Test) {
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_ceval_literals.candi");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();
		// Create the runtime namespace/scope/environment.
		// For clarity: - All three have the same meaning in this context,but we will use the term "environment".
		//              - Namespace would clash with C++ namespace, and scope is already used in the parser.
		//              - Each environment has a parent environment, which is nullptr for the root environment(global scope).
		//              - Each environment also has a list of sub-environments, which are the child scopes.
		auto runtime_env = caoco::rtenv("global");


		// Test the constant evaluator on literals
		auto int_literal = caoco::parse_operand(result.cbegin(), result.cend());
		auto eval_result = caoco::CNumberEval()(int_literal.expected(), runtime_env);
		EXPECT_EQ(eval_result.type, caoco::RTValue::eType::NUMBER);
		EXPECT_EQ(std::get<int>(eval_result.value), 42);

		// real literal
		auto real_literal = caoco::parse_operand(int_literal.always(), result.cend());
		eval_result = caoco::CRealEval()(real_literal.expected(), runtime_env);
		EXPECT_EQ(eval_result.type, caoco::RTValue::eType::REAL);
		EXPECT_EQ(std::get<double>(eval_result.value), 42.42);

		// string literal
		auto string_literal = caoco::parse_operand(real_literal.always(), result.cend());
		eval_result = caoco::CStringEval()(string_literal.expected(), runtime_env);
		EXPECT_EQ(eval_result.type, caoco::RTValue::eType::STRING);
		EXPECT_EQ(std::get<caoco::sl_string>(eval_result.value), "Hello'World");

		// bit literal
		auto bit_literal = caoco::parse_operand(string_literal.always(), result.cend());
		eval_result = caoco::CBitEval()(bit_literal.expected(), runtime_env);
		EXPECT_EQ(eval_result.type, caoco::RTValue::eType::BIT);
		EXPECT_EQ(std::get<bool>(eval_result.value), true);

		// unsigned int literal
		auto uint_literal = parse_operand(bit_literal.always(), result.cend());
		eval_result = caoco::CUnsignedEval()(uint_literal.expected(), runtime_env);
		EXPECT_EQ(eval_result.type, caoco::RTValue::eType::UNSIGNED);
		EXPECT_EQ(std::get<unsigned int>(eval_result.value), 42u);

		// octet literal
		auto octet_literal = parse_operand(uint_literal.always(), result.cend());
		eval_result = caoco::COctetEval()(octet_literal.expected(), runtime_env);
		EXPECT_EQ(eval_result.type, caoco::RTValue::eType::BYTE);
		EXPECT_EQ(std::get<unsigned char>(eval_result.value), (unsigned char)42);

		// octet  from char
		auto octet_from_char = parse_operand(octet_literal.always(), result.cend());
		eval_result = caoco::COctetEval()(octet_from_char.expected(), runtime_env);
		EXPECT_EQ(eval_result.type, caoco::RTValue::eType::BYTE);
		EXPECT_EQ(std::get<unsigned char>(eval_result.value), (unsigned char)'a');

		// none
		auto none_literal = parse_operand(octet_from_char.always(), result.cend());
		eval_result = caoco::CNoneEval()(none_literal.expected(), runtime_env);
		EXPECT_EQ(eval_result.type, caoco::RTValue::eType::NONE);
		EXPECT_EQ(std::get<caoco::none_t>(eval_result.value), caoco::none_t{});

	}
	else
	{
		std::cout << exp_result.error_message() << std::endl;
	}
}
#endif

#if CAOCO_TEST_CONST_EVALUATOR_MathOperators
TEST(CaocoConstantEvaluator_Operators, CaocoConstantEvaluator_Operators) {
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_ceval_operators.candi");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();
		auto runtime_env = caoco::rtenv("global");

		// operator <numlit><+><numlit> 1+1
		auto expr = caoco::parse_value_statement(result.cbegin(), result.cend());
		auto eval_result = caoco::CAddOpEval()(expr.expected(), runtime_env);
		EXPECT_EQ(eval_result.type, caoco::RTValue::eType::NUMBER);
		EXPECT_EQ(std::get<int>(eval_result.value), 2);

		// multiple operators <numlit><+><numlit><+><numlit> 1+1+1
		expr = caoco::parse_value_statement(expr.always(), result.cend());
		eval_result = caoco::CBinopEval()(expr.expected(), runtime_env);
		EXPECT_EQ(eval_result.type, caoco::RTValue::eType::NUMBER);
		EXPECT_EQ(std::get<int>(eval_result.value), 5);


		// operator -        1 + 1 - 1
		expr = caoco::parse_value_statement(expr.always(), result.cend());
		eval_result = caoco::CBinopEval()(expr.expected(), runtime_env);
		EXPECT_EQ(eval_result.type, caoco::RTValue::eType::NUMBER);
		EXPECT_EQ(std::get<int>(eval_result.value), 1);

		//// operators + - * / %
		//// 1 + 1 - 1 * 1 / 1 % 1 (== 2)
		//expr = caoco::parse_value_statement(expr.always(), result.cend());
		//eval_result = caoco::CBinopEval()(expr.expected(), runtime_env);
		//EXPECT_EQ(eval_result.type, caoco::RTValue::eType::NUMBER);
		//EXPECT_EQ(std::get<int>(eval_result.value), 2);

		// variable in expression
		//1 + a;
		runtime_env.create_variable("a", caoco::RTValue(caoco::RTValue::eType::NUMBER, 42));
		expr = caoco::parse_value_statement(expr.always(), result.cend());
		eval_result = caoco::CBinopEval()(expr.expected(), runtime_env);
		EXPECT_EQ(eval_result.type, caoco::RTValue::eType::NUMBER);
		EXPECT_EQ(std::get<int>(eval_result.value), 43);

	}
	else
	{
		std::cout << exp_result.error_message() << std::endl;
	}
}
#endif

#if CAOCO_TEST_CONST_EVALUATOR_VariableDeclaration
TEST(CaocoConstantEvaluator_VariableDeclaration, CaocoConstantEvaluator_VariableDeclaration) {
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_ceval_vardecl.candi");
	auto exp_result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	EXPECT_TRUE(exp_result.valid());
	if (exp_result.valid()) {
		auto result = exp_result.expected();
		auto runtime_env = caoco::rtenv("global");

		// #var a = 1;
		auto var_decl = caoco::parse_directive_var(result.cbegin(), result.cend());
		//print_ast(var_decl.node());
		EXPECT_TRUE(var_decl.valid());
		auto eval_result = caoco::CVarDeclEval()(var_decl.expected(), runtime_env);
		EXPECT_EQ(eval_result.type, caoco::RTValue::eType::NUMBER);
		EXPECT_EQ(std::get<int>(eval_result.value), 1);
		//std::cout << "Variable a = " << std::get<int>(runtime_env.resolve_variable("a").value().value) << std::endl;
		EXPECT_EQ(std::get<int>(runtime_env.resolve_variable("a").value().value), 1);
	}
	else {
		std::cout << exp_result.error_message() << std::endl;
	}
}
#endif

#if CAOCO_UT_ConstantEvaluator_Structs
TEST(CaocoConstantEvaluator_Structs, CaocoConstantEvaluator_Test) {
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_ceval_structs.candi");
	auto result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	auto runtime_env = caoco::rtenv("global");

	/* Basic class with variable members
		#class Foo {
			#var a = 1;
			#var b = 2;
		};
	*/

	//caoco::ParseDirectiveClass()
	auto class_decl = caoco::ParseDirectiveClass()(result.cbegin(), result.cend());
	EXPECT_TRUE(class_decl.valid());
	print_ast(class_decl.node());
	auto eval_result = caoco::CClassDeclEval()(class_decl.node(), runtime_env);

	auto class_obj = std::get<std::shared_ptr<caoco::object_t>>(eval_result.value).get();
	EXPECT_EQ(std::get<int>(class_obj->get_member("a").value), 1);
	EXPECT_EQ(std::get<int>(class_obj->get_member("b").value), 2);
	EXPECT_EQ(std::get<int>(class_obj->get_member("c").value), 3);
}
#endif
#if CAOCO_UT_ConstantEvaluator_FreeFunctions
TEST(CaocoConstantEvaluator_FreeFunctions, CaocoConstantEvaluator_Test) {
	auto source_file = caoco::sl::load_file_to_char8_vector("ut_ceval_free_functions.candi");
	auto result = caoco::tokenizer(source_file.cbegin(), source_file.cend())();
	auto runtime_env = caoco::rtenv("global");

	/*
		#func add(x) {
			#return x + 40;
		};
		add(2);
	*/
	auto func_decl = caoco::ParseDirectiveFunc()(result.cbegin(), result.cend());
	EXPECT_TRUE(func_decl.valid());
	print_ast(func_decl.node());
	auto eval_result = caoco::CFunctionDeclEval()(func_decl.node(), runtime_env);

	// Check if function was declared in global scope.
	auto func_obj_from_env = runtime_env.resolve_variable("add");

	// check type of function object
	EXPECT_EQ(func_obj_from_env.value().type, caoco::RTValue::eType::FUNCTION);


	//// Call the function.
	//auto func_call = caoco::ParseValueExpression()(func_decl.it(), result.cend());
	//EXPECT_TRUE(func_call.valid());
	//print_ast(func_call.node());
	//auto eval_result2 = caoco::CFunctionCallEval()(func_call.node(), runtime_env);

	//// Check the result of the function call.
	//EXPECT_EQ(eval_result2.type, caoco::RTValue::eType::NUMBER);
	//EXPECT_EQ(std::get<int>(eval_result2.value), 42);

}
#endif
