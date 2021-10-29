section("functions", (test) => {
  test("function returns expected value", () => {
    function func() {
      return 123;
    }
    const result = func();
    assert(result == 123, "expected: 123, got: " + result);
  });
});
