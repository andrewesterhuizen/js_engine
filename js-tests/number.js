section("number", (test) => {
  test("hex literal returns number", () => {
    assert(0x1 == 1, "expected result to be true");
    assert(0xff == 255, "expected result to be true");
  });

  test("== operation returns expected result", () => {
    assert(123 == 123, "expected result to be true");
  });

  test("!= operation returns expected result", () => {
    assert(123 != 124, "expected result to be true");
  });

  test("parseInt returns expected value", () => {
    const result = parseInt("123");
    assert(result === 123, "expected 123, got " + result);
  });

  test("parseInt returns expected value with decimal input", () => {
    const result = parseInt("123.5");
    assert(result === 123, "expected 123, got " + result);
  });

  test("parseInt works with number input", () => {
    const result = parseInt(123);
    assert(result === 123, "expected 123, got " + result);
  });

  test("parseFloat returns expected value", () => {
    const result = parseFloat("123");
    assert(result === 123, "expected 123, got " + result);
  });

  test("parseFloat returns expected value with decimal input", () => {
    const result = parseFloat("123.5");
    assert(result === 123.5, "expected 123.5, got " + result);
  });

  test("parseFloat works with number input", () => {
    const result = parseFloat(123.5);
    assert(result === 123.5, "expected 123.5, got " + result);
  });

  const bitwiseOrTestCases = [
    { name: "0 | 1", value: 0 | 1, result: 1 },
    { name: "1 | 1", value: 1 | 1, result: 1 },
    { name: "1 | 0", value: 1 | 0, result: 1 },
    { name: "0 | 0", value: 0 | 0, result: 0 },
  ];

  test("biwise or returns expected result", () => {
    bitwiseOrTestCases.forEach((testCase) => {
      assert(
        testCase.value == testCase.result,
        "expected " +
          testCase.name +
          " to equal " +
          testCase.result +
          " but got " +
          testCase.value
      );
    });
  });

  const bitwiseAndTestCases = [
    { name: "0 & 1 ->", value: 0 & 1, result: 0 },
    { name: "1 & 1 ->", value: 1 & 1, result: 1 },
    { name: "1 & 0 ->", value: 1 & 0, result: 0 },
    { name: "0 & 0 ->", value: 0 & 0, result: 0 },
  ];

  test("biwise and returns expected result", () => {
    bitwiseAndTestCases.forEach((testCase) => {
      assert(
        testCase.value == testCase.result,
        "expected " +
          testCase.name +
          " to equal " +
          testCase.result +
          " but got " +
          testCase.value
      );
    });
  });
});
