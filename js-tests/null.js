section("null", (test) => {
  test("null literal", () => {
    assert(null === null, "null literal should should be equal to itself");
  });

  test("null variable is null", () => {
    var x = null;
    assert(x === null, "variable should should be equal to null");
  });
});
