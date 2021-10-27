console.log(this);

var x = 123;

console.log(x);
console.log(this.x);

function f() {
    console.log("f called");
    this.y = 234;
}

f();

console.log("y:", y);
