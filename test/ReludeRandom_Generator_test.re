open Jest;
open Expect;

open ReludeRandom;

/**
 * Testing randomness seems like a real challenge... Maybe property-based tests
 * would be better? I at least want some sanity checks to make sure the API
 * works as advertised and produces evenly-distributed values.
 */
describe("Generator", () => {
  test("int with constant seed", () => {
    Generator.int(~min=1, ~max=100)
    |> Generator.step(_, Seed.fromInt(42))
    |> fst
    |> expect
    |> toEqual(39)
  })
});

describe("RandomList", () => {
  test("list with fixed size", () =>
    RandomList.make(~length=7, Generator.int(~min=-10, ~max=10))
    |> Generator.step(_, Seed.fromInt(4))
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
    |> Generator.step(_, Seed.fromInt(888888))
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
    |> Generator.step(_, Seed.fromInt(1))
    |> fst
    |> expect
    |> toEqual([])
  );
});
