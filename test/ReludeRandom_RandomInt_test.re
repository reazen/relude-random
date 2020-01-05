open Jest;
open Expect;

open Relude.Globals;
open ReludeRandom;

describe("RandomInt", () => {
  let diceGen = RandomInt.make(~min=1, ~max=6);

  test("values are distributed evenly", () => {
    let seed = Seed.fromInt(987654);
    let diceGen = RandomInt.make(~min=1, ~max=6);
    TestUtil.testDistribution((module Int.Ord), ~samples=60000, diceGen, seed)
    |> expect
    |> toEqual([
         (1, 10087),
         (2, 9968),
         (3, 10053),
         (4, 9815),
         (5, 9907),
         (6, 10170),
       ]);
  });

  test("values are distributed evenly (different seed)", () => {
    let seed = Seed.fromInt(71);
    TestUtil.testDistribution((module Int.Ord), ~samples=60000, diceGen, seed)
    |> expect
    |> toEqual([
         (1, 9789),
         (2, 10145),
         (3, 10095),
         (4, 9938),
         (5, 9986),
         (6, 10047),
       ]);
  });

  test("make (min/max flipped)", () =>
    RandomInt.make(~min=5, ~max=-5)
    |> Generator.run(_, Seed.fromInt(-644910))
    |> fst
    |> expect
    |> toEqual(-4)
  );

  TestUtil.testAllRandomSeed(
    "anyPositive",
    v => v > 1,
    RandomInt.anyPositive,
  );

  TestUtil.testAllRandomSeed(
    "anyNegative",
    v => v < 0,
    RandomInt.anyNegative,
  );

  TestUtil.testAllRandomSeed(
    "greaterThan",
    v => v > 8912223,
    RandomInt.greaterThan(8912223),
  );

  TestUtil.testAllRandomSeed(
    "lessThan",
    v => v < (-675113),
    RandomInt.lessThan(-675113),
  );

  TestUtil.testAllRandomSeed(
    "fromZeroTo (positive)",
    v => v >= 0 && v <= 8,
    RandomInt.fromZeroTo(8),
  );

  TestUtil.testAllRandomSeed(
    "fromZeroTo (negative)",
    v => v <= 0 && v >= (-16),
    RandomInt.fromZeroTo(-16),
  );
});
