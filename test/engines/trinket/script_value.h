#include <cxxtest/TestSuite.h>
#include <stdint.h>

#include "trinket/core/object.h"
#include "trinket/core/script_runtime.h"
#include "trinket/string_parser.h"

class TrinketScriptValueTestSuite : public CxxTest::TestSuite {
public:
	/** Numeric classification retains text used by native decimal-prefix consumers. */
	void testNumericText() {
		const Trinket::Core::ScriptValue value = Trinket::Core::ScriptValue::fromText("1e3");
		TS_ASSERT_EQUALS(value.kind(), Trinket::Core::ScriptValue::kInteger);
		TS_ASSERT_EQUALS(value.toFloat64(), 1000);
		TS_ASSERT_EQUALS(value.toInt32Rounded(), 1000);
		TS_ASSERT_EQUALS(value.toInt32(), 1);
		TS_ASSERT_EQUALS(value.toText(), "1e3");
		const Trinket::Core::ScriptValue prefix = Trinket::Core::ScriptValue::fromText("7suffix");
		TS_ASSERT_EQUALS(prefix.toFloat64(), 7);
		TS_ASSERT_EQUALS(prefix.toText(), "7suffix");
	}

	/** Reduction obeys both the native magnitude bound and the original text length. */
	void testReductionLimits() {
		using Trinket::Core::ScriptValue;
		TS_ASSERT_EQUALS(ScriptValue::fromText("2000000000").kind(), ScriptValue::kInteger);
		TS_ASSERT_EQUALS(ScriptValue::fromText("2000000001").kind(), ScriptValue::kReal);
		TS_ASSERT_EQUALS(ScriptValue::fromText("-2000000000").kind(), ScriptValue::kReal);
		TS_ASSERT_EQUALS(ScriptValue::fromText("000000000001").kind(), ScriptValue::kReal);
		TS_ASSERT_EQUALS(ScriptValue::fromText("000000000001").toFloat64(), 1);
		TS_ASSERT_EQUALS(ScriptValue(-2000000000.0).kind(), ScriptValue::kInteger);
		TS_ASSERT_EQUALS(ScriptValue(1.0).kind(), ScriptValue::kInteger);
		TS_ASSERT_EQUALS(ScriptValue(0.25).kind(), ScriptValue::kReal);
	}

	/** Compiled real literals retain their serialized type and explicitly empty text. */
	void testCompiledLiterals() {
		using Trinket::Core::ScriptValue;
		const ScriptValue value = ScriptValue::fromLiteral(1.0, "1.0");
		TS_ASSERT_EQUALS(value.kind(), ScriptValue::kReal);
		TS_ASSERT_EQUALS(value.toFloat64(), 1);
		TS_ASSERT_EQUALS(value.toText(), "1.0");
		TS_ASSERT_EQUALS(ScriptValue::fromLiteral(1, "001").toText(), "001");
		TS_ASSERT(ScriptValue::fromLiteral(0.0, "").toText().empty());
		TS_ASSERT(ScriptValue::fromLiteral(17, "").toText().empty());
	}

	/** VM integer reads round to nearest while integer arithmetic truncates. */
	void testNumericIntegerConversions() {
		using Trinket::Core::ScriptValue;
		TS_ASSERT_EQUALS(ScriptValue(2.75).toInt32Rounded(), 3);
		TS_ASSERT_EQUALS(ScriptValue(2.5).toInt32Rounded(), 2);
		TS_ASSERT_EQUALS(ScriptValue(-2.5).toInt32Rounded(), -3);
		TS_ASSERT_EQUALS(ScriptValue(-2.75).toInt32Rounded(), -3);
		TS_ASSERT_EQUALS(ScriptValue(2.75).toInt32Trunc(), 2);
		TS_ASSERT_EQUALS(ScriptValue(-2.75).toInt32Trunc(), -2);
		TS_ASSERT_EQUALS(ScriptValue(2.75).toInt32(), 2);
		TS_ASSERT_EQUALS(ScriptValue(Common::String("2.75")).toInt32Rounded(), 0);
		TS_ASSERT_EQUALS(ScriptValue(Common::String("2.75")).toFloat64(), 0);
	}

	/** Native parameters parse decimal prefixes with the original signed 32-bit wrapping. */
	void testNativeParameterPrefixes() {
		using Trinket::StringParser;
		TS_ASSERT_EQUALS(StringParser::parseInt32Prefix("  -12tail"), -12);
		TS_ASSERT_EQUALS(StringParser::parseInt32Prefix("1e3"), 1);
		TS_ASSERT_EQUALS(StringParser::parseInt32Prefix("0x10"), 0);
		TS_ASSERT_EQUALS(StringParser::parseInt32Prefix("2147483648"), INT32_MIN);
		TS_ASSERT_EQUALS(StringParser::parseInt32Prefix("-2147483649"), INT32_MAX);
		TS_ASSERT_EQUALS(StringParser::parseDoublePrefix("1.25tail"), 1.25);
		TS_ASSERT_EQUALS(StringParser::parseDoublePrefix("0x10"), 0);
		TS_ASSERT_EQUALS(StringParser::parseDoublePrefix("invalid"), 0);
	}

	/** Variable assignment reclassifies incoming text while preserving its spelling. */
	void testVariableAssignment() {
		using Trinket::Core::ScriptValue;
		Trinket::Core::ScriptRuntime runtime;
		runtime.setVariable("value", ScriptValue(Common::String("1e3")));
		TS_ASSERT_EQUALS(runtime.variable("value").kind(), ScriptValue::kInteger);
		TS_ASSERT_EQUALS(runtime.variable("value").toFloat64(), 1000);
		TS_ASSERT_EQUALS(runtime.variable("value").toText(), "1e3");
		runtime.setVariable("value", ScriptValue(-2000000000.0));
		TS_ASSERT_EQUALS(runtime.variable("value").kind(), ScriptValue::kReal);
		TS_ASSERT_EQUALS(runtime.variable("value").toFloat64(), -2000000000.0);
	}

	/** Prefix compatibility does not relax the separate checked parsing API. */
	void testCheckedParserRemainsStrict() {
		using Trinket::StringParser;
		double value = 19;
		TS_ASSERT_EQUALS(StringParser::parseDouble("7suffix", value, StringParser::kDecimalProfile), StringParser::Result::kTrailingCharacters);
		TS_ASSERT_EQUALS(value, 19);
		TS_ASSERT_EQUALS(StringParser::parseDouble("1e9999", value, StringParser::kDecimalProfile), StringParser::Result::kOutOfRange);
		TS_ASSERT_EQUALS(value, 19);
	}
};
