function MyClass() {}
var instance = new MyClass();
console.log(instance.toString());
console.log(({}));
console.log(({}).toString());

function MessageLogger(message) {
    this.message = message;
}

MessageLogger.prototype.log = function() {
    return console.log(this.message);
};

MessageLogger.prototype.toString = function() {
    return this.message;
};

MessageLogger.prototype.toString = function() {
    return "MessageLogger with message: " + this.message;
};



const logger = new MessageLogger("my message");
logger.log();
console.log(logger.toString());


try {
asdf.y = 1;
} catch(error) {
    console.log(error.name == "ReferenceError");
}
throw new Error("my error message");

