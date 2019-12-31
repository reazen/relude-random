module Generator = ReludeRandom_Generator;

let make = Generator.int;

let any = make(~min=min_int, ~max=max_int);

let anyPositive = make(~min=1, ~max=max_int);

let anyNegative = make(~min=min_int, ~max=-1);

let greaterThan = n => make(~min=n + 1, ~max=max_int);

let lessThan = n => make(~min=min_int, ~max=n - 1);

let fromZeroTo = max => make(~min=0, ~max);
