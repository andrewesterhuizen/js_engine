section("undefined", (test) => {
  test("undefined literal", () => {
    assert(
      undefined === undefined,
      "undefined literal should should be equal to itself"
    );
  });

  test("variable declared with no value is undefined", () => {
    var x;
    assert(x === undefined, "variable should should be equal to undefined");
  });
});
