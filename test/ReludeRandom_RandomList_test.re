open Jest;
open Expect;

open ReludeRandom;

describe("RandomList", () => {
  test("list with fixed size", () =>
    RandomList.make(~length=7, Generator.int(~min=-10, ~max=10))
    |> Generator.run(_, Seed.fromInt(4))
    |> fst
    |> expect
    |> toEqual([(-8), (-4), (-5), (-4), 4, 7, 4])
  );

  test("list with variable size", () =>
    RandomList.randomLength(
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
    RandomList.randomLength(
      ~minLength=-20,
      ~maxLength=-10,
      Generator.int(~min=0, ~max=2),
    )
    |> Generator.run(_, Seed.fromInt(1))
    |> fst
    |> expect
    |> toEqual([])
  );
});
