function MessageLogger(message) {
    this.message = message;

    this.logMessage = function() {
        console.log(this.message);
    };
}


const instance = new MessageLogger("hello from instance of class");
instance.logMessage();
console.log(instance);