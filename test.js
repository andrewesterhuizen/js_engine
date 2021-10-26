var a = ["hello", "from", "array", "forEach"];
a.push("!");

var logString = function (s) {
  console.log(s);
};

a.forEach(logString);

var b = [1, 2, 3, 4, 5];
b.pop();

var c = b.map(function (n) {
  return n + 1;
});

console.log(b);
console.log(c);

var d = [true, false, true, true, false];
console.log(
  d.filter(function (v) {
    return v;
  })
);

console.log(
  b.reduce(function (prev, current) {
    return prev + current;
  }, 0)
);
