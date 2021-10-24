function euclideanAlgorithm(originalA, originalB) {
  // Make input numbers positive.
  const a = Math.abs(originalA);
  const b = Math.abs(originalB);

  // To make algorithm work faster instead of subtracting one number from the other
  // we may use modulo operation.
  return (b === 0) ? a : euclideanAlgorithm(b, a % b);
}

function leastCommonMultiple(a, b) {
  return ((a === 0) || (b === 0)) ? 0 : Math.abs(a * b) / euclideanAlgorithm(a, b);
}

console.log(leastCommonMultiple(5,4));