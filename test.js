(() => console.log("hello from lambda"))();

var a = ["hello", "from", "array", "forEach"];
a.push("!");

a.forEach((s) => console.log(s));

var b = [1, 2, 3, 4, 5];
b.pop();

var c = b.map((n => n + 1));

console.log(b);
console.log(c);

var d = [true, false, true, true, false];
console.log(d.filter((v) => v));

console.log(b.reduce((prev, current) => prev + current, 0));
