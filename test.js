var a = [1, 2, 3, 4];
var b = Array.from(a);
console.log(b);

var c = Array.from(a, (x) => x + 1);
console.log(c);
