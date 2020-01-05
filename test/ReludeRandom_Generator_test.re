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
    |> Generator.run(_, Seed.fromInt(42))
    |> fst
    |> expect
    |> toEqual(39)
  });

  test("weighted (expected distribution)", () => {
    let weekdayGen =
      Generator.weighted(
        (10.0, TestUtil.Weekday.Sun),
        [
          (1.0, Mon),
          (1.0, Tue),
          (1.0, Wed),
          (1.0, Thu),
          (2.0, Fri),
          (4.0, Sat),
        ],
      );

    TestUtil.testDistribution(
      (module TestUtil.Weekday),
      ~samples=20000,
      weekdayGen,
      Seed.fromInt(-744982),
    )
    |> expect
    |> toEqual(
         TestUtil.Weekday.[
           (Sun, 9890),
           (Mon, 1048),
           (Tue, 969),
           (Wed, 970),
           (Thu, 964),
           (Fri, 2007),
           (Sat, 4152),
         ],
       );
  });

  TestUtil.testAllRandomSeed(
    "sample (is some for non-empty list)",
    Relude.Option.isSome,
    RandomList.randomLength(~minLength=1, ~maxLength=20, RandomInt.any)
    |> Generator.flatMap(Generator.sample),
  );

  test("sample (empty list is none", () =>
    Generator.sample([])
    |> Generator.run(_, Seed.fromInt(3))
    |> fst
    |> expect
    |> toEqual(None)
  );

  TestUtil.testAllRandomSeed(
    "pure (always returns same value)",
    v => v == "xxzz",
    Generator.pure("xxzz"),
  );

  test("fromEnum", () =>
    TestUtil.testDistribution(
      (module TestUtil.Weekday),
      Generator.fromEnum((module TestUtil.Weekday), ~min=Mon, ~max=Wed),
      Seed.fromInt(-761761),
    )
    |> expect
    |> toEqual(TestUtil.Weekday.[(Mon, 3346), (Tue, 3339), (Wed, 3315)])
  );

  test("fromBoundedEnum", () =>
    TestUtil.testDistribution(
      (module TestUtil.Weekday),
      Generator.fromBoundedEnum((module TestUtil.Weekday)),
      Seed.fromInt(0),
    )
    |> expect
    |> toEqual(
         TestUtil.Weekday.[
           (Sun, 1431),
           (Mon, 1463),
           (Tue, 1435),
           (Wed, 1369),
           (Thu, 1430),
           (Fri, 1455),
           (Sat, 1417),
         ],
       )
  );
});
