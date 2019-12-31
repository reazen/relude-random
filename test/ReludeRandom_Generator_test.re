open Jest;
open Expect;

open Relude.Globals;
open ReludeRandom;

// describe("Seed", () => {
//   test("construct a seed", () => {
//     let v = Seed.fromInt(42);
//     expect(v) |> toEqual(Seed.Seed(1266345812, 1013904223));
//   });
//   test("peel an int from a seed", () => {
//     let v = Seed.peel(Seed.fromInt(42));
//     expect(v) |> toEqual(1298916341);
//   });
// });

/**
 * Testing randomness seems like a real challenge... Maybe property-based tests
 * would be better? I at least want some sanity checks to make sure the API
 * works as advertised and produces evenly-distributed values.
 */
describe("Generator", () => {
  test("int with constant seed", () => {
    Generator.int(~min=1, ~max=100)
    |> Generator.run(_, Seed.fromInt(42))
    |> fst
    |> expect
    |> toEqual(39)
  })
});

describe("RandomList", () => {
  test("list with fixed size", () =>
    RandomList.make(~length=7, Generator.int(~min=-10, ~max=10))
    |> Generator.run(_, Seed.fromInt(4))
    |> fst
    |> expect
    |> toEqual([(-8), (-4), (-5), (-4), 4, 7, 4])
  );

  test("list with variable size", () =>
    RandomList.makeRandomLength(
      ~minLength=3,
      ~maxLength=19,
      Generator.float(~min=0.0, ~max=1.0),
    )
    |> Generator.run(_, Seed.fromInt(888888))
    |> fst
    |> List.length
    |> expect
    |> toEqual(7)
  );

  test("list with negative size", () =>
    RandomList.makeRandomLength(
      ~minLength=-20,
      ~maxLength=-10,
      Generator.int(~min=0, ~max=2),
    )
    |> Generator.run(_, Seed.fromInt(1))
    |> fst
    |> expect
    |> toEqual([])
  );

  let collectCounts =
    List.foldLeft(
      (acc, x) =>
        Int.Map.set(
          x,
          Option.fold(1, v => v + 1, Int.Map.get(x, acc)),
          acc,
        ),
      Relude.Int.Map.make(),
    );

  test("values are distributed evenly", () => {
    let seed = Seed.fromInt(987654);
    let diceGen = RandomInt.make(~min=1, ~max=6);
    RandomList.make(~length=60000, diceGen)
    |> Generator.run(_, seed)
    |> fst
    |> collectCounts
    |> Int.Map.toList
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
    let diceGen = RandomInt.make(~min=1, ~max=6);
    RandomList.make(~length=60000, diceGen)
    |> Generator.run(_, seed)
    |> fst
    |> collectCounts
    |> Int.Map.toList
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
});
