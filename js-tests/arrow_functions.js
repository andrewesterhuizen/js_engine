section("arrow functions", (test) => {
  test("arrow function returns expression value when body is single expression", () => {
    const func = () => 123;
    const result = func();
    assert(result == 123, "expected: 123, got: " + result);
  });
});
