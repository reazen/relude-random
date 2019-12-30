module Seed = ReludeRandom_Seed;
module Generator = ReludeRandom_Generator;

let rec listHelp:
  (list('a), int, Seed.t => ('a, Seed.t), Seed.t) => (list('a), Seed.t) =
  (acc, n, gen, seed) =>
    if (n < 1) {
      (acc, seed);
    } else {
      let (value, newSeed) = gen(seed);
      listHelp([value, ...acc], n - 1, gen, newSeed);
    };

let make = (~length: int, Generator.Generator(gen)) =>
  Generator.Generator(seed => listHelp([], length, gen, seed));

let makeRandomLength = (~minLength: int, ~maxLength: int, gen) =>
  Generator.int(~min=minLength < 0 ? 0 : minLength, ~max=maxLength)
  |> Generator.flatMap(length => make(~length, gen));
