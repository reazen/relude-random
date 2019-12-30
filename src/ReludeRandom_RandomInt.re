module Generator = ReludeRandom_Generator;

let inRange = Generator.int;

let any = inRange(~min=min_int, ~max=max_int);

let anyPositive = inRange(~min=1, ~max=max_int);

let anyNegative = inRange(~min=min_int, ~max=-1);

let greaterThan = n => inRange(~min=n + 1, ~max=max_int);

let lessThan = n => inRange(~min=min_int, ~max=n - 1);
