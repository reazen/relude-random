open Jest;
open Expect;

open ReludeRandom;

/**
 * Testing randomness seems like a real challenge... Maybe property-based tests
 * would be better? I at least want some sanity checks to make sure the API
 * works as advertised and produces evenly-distributed values.
 */
describe("ReludeRandom.Generator", () => {
  test("int with constant seed", () => {
    Generator.int(~min=1, ~max=100)
    |> Generator.step(_, Seed.fromInt(42))
    |> fst
    |> expect
    |> toEqual(39)
  })
});
