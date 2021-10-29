const testCases = [
  { name: "undefined", value: undefined, result: "undefined" },
  { name: "number", value: 1, result: "number" },
  { name: "boolean: true", value: true, result: "boolean" },
  { name: "boolean: false", value: false, result: "boolean" },
  { name: "null", value: null, result: "object" },
  { name: "string: empty", value: "", result: "string" },
  { name: "string: empty", value: "a string", result: "string" },
  { name: "function: arrow", value: () => {}, result: "function" },
  { name: "function", value: function () {}, result: "function" },
  { name: "object", value: {}, result: "object" },
];

section("typeof", (test) => {
  for (let i = 0; i < testCases.length; i++) {
    const testCase = testCases[i];

    test(testCase.name + ' -> "' + testCase.result + '"', () => {
      const result = typeof testCase.value;
      assert(
        result === testCase.result,
        "expected: " + testCase.result + ", got: " + result
      );
    });
  }
});
