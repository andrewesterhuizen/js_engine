function throws_error() {
    throw "error throw in function";
}

function start() {
    try {
        throws_error();
    } catch(fn_error) {
        console.log("error caught in function:");
        console.log(fn_error);
        console.log("rethrowing");
        throw fn_error;
    }
}

try {
    console.log("hello from inside top level try block");
    start();
} catch(error) {
    console.log("error caught in top level catch:" );
    console.log(error);
}




console.log("end");

