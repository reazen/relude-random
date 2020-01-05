open Jest;
open Expect;

open Relude.Globals;
open ReludeRandom;

module BoolOrd = {
  type t = bool;
  let eq = (a: bool, b: bool) => a == b;
  let compare = (a: bool, b: bool) => Ordering.fromInt(compare(a, b));
};

describe("RandomBool", () => {
  test("generator (choose one at random)", () =>
    TestUtil.testDistribution(
      (module BoolOrd),
      RandomBool.generator,
      Seed.fromInt(12),
    )
    |> expect
    |> toEqual([(false, 4999), (true, 5001)])
  );

  test("oneIn (100 is ~1%)", () =>
    TestUtil.testDistribution(
      (module BoolOrd),
      RandomBool.oneIn(100),
      Seed.fromInt(12345678),
    )
    |> expect
    |> toEqual([(false, 9901), (true, 99)])
  );

  test("oneIn (4 is ~25%)", () =>
    TestUtil.testDistribution(
      (module BoolOrd),
      RandomBool.oneIn(4),
      Seed.fromInt(-987654),
    )
    |> expect
    |> toEqual([(false, 7437), (true, 2563)])
  );
});
