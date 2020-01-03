module Generator = ReludeRandom_Generator;

let generator = Generator.choose(true, false);

let oneIn = n =>
  ReludeRandom_RandomInt.make(~min=1, ~max=abs(n))
  |> Generator.map(v => v == 1);
