section("array", (test) => {
  test("array length returns expexted length", () => {
    var a = [1, 2, 3, 4];
    assert(a.length === 4, "expected array length to be 4 and got " + a.length);
  });

  test("array indexing returns expected value", () => {
    var a = [123];

    assert(a[0] === 123, "expected value to be 123 and got " + a[0]);
  });

  test("array.push pushes value", () => {
    var a = [];
    a.push(123);

    assert(a[0] === 123, "expected pushed value to be 123 and got " + a[0]);
  });
});
