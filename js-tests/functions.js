section("functions", (test) => {
  test("function returns expected value", () => {
    function func() {
      return 123;
    }
    const result = func();
    assert(result == 123, "expected: 123, got: " + result);
  });

  test("function has arguments object", () => {
    function func() {
      assert(
        arguments.length === 3,
        "expected length to be 3, got: " + arguments.length
      );
      assert(arguments[0] === 1, "expected: 1, got: " + arguments[0]);
      assert(arguments[1] === 2, "expected: 2, got: " + arguments[1]);
      assert(arguments[2] === 3, "expected: 3, got: " + arguments[2]);
    }

    func(1, 2, 3);
  });
});
