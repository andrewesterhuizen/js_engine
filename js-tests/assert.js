function AssertError(message) {
  this.name = "AssertError";
  this.message = message;
}
AssertError.prototype = Error.prototype;

const assert = (result, message) => {
  if (!result) {
    throw new AssertError(message);
  }
};

const section = (sectionName, executeSection) => {
  const results = [];

  const test = (testName, executeTest) => {
    try {
      executeTest();
    } catch (error) {
      if (error.name == "AssertError") {
        results.push({
          test: testName,
          passed: false,
          message: error.message,
        });
        return;
      }

      throw error;
    }

    results.push({
      test: testName,
      passed: true,
    });
  };

  executeSection(test);

  console.log({
    section: sectionName,
    results: results,
  });
};
