section("number", (test) => {
  test("== operation returns expected result", () => {
    assert(123 == 123, "expected result to be true");
  });

  test("!= operation returns expected result", () => {
    assert(123 != 124, "expected result to be true");
  });
});
