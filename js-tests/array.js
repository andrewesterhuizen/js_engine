section("array", (test) => {
  test("Array constructor returns array with expected length", () => {
    var a = new Array(5);
    assert(a.length == 5, "expected: 5, got: " + a.length);
  });

  test("array.fill() fills array with value", () => {
    var length = 5;
    var a = new Array(5);
    a.fill(123);

    for (var i = 0; i < length; i++) {
      assert(a[i] == 123, "expected: 123, got: " + a[i] + ", at position " + i);
    }
  });

  test("Array.from returns new array with copy of elements", () => {
    var a = [1, 2, 3, 4];
    var b = Array.from(a);

    for (var i = 0; i < a.length; i++) {
      assert(
        a[i] === b[i],
        "expected: " + a[i] + ", got: " + b[i] + ", at position " + i
      );
    }
  });

  test("Array.from execute map function", () => {
    var a = [1, 2, 3, 4];
    var b = Array.from(a, (x) => x + 1);

    for (var i = 0; i < a.length; i++) {
      var expected = a[i] + 1;
      assert(
        expected === b[i],
        "expected: " + expected + ", got: " + b[i] + ", at position " + i
      );
    }
  });

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
