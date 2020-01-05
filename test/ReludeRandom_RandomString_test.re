open Jest;
open Expect;

open ReludeRandom;

describe("RandomString", () => {
  test("make (uppercase plus fixed-length lowercase", () =>
    Generator.map2(
      (++),
      RandomString.upperCaseLatinChar,
      RandomString.makeLowerCaseLatin(~length=4),
    )
    |> Generator.run(_, Seed.fromInt(-7833209))
    |> fst
    |> expect
    |> toEqual("Tybai")
  );

  test("make (negative length is empty)", () =>
    RandomString.make(~length=-20, RandomString.lowerCaseLatinChar)
    |> Generator.run(_, Seed.fromInt(76543))
    |> fst
    |> expect
    |> toEqual("")
  );

  test("makeLatin", () =>
    RandomString.makeLatin(~length=6)
    |> Generator.run(_, Seed.fromInt(6123139))
    |> fst
    |> expect
    |> toEqual("yjMjie")
  );

  test("makeUpperCaseLatin", () =>
    RandomString.makeUpperCaseLatin(~length=16)
    |> Generator.run(_, Seed.fromInt(-109008))
    |> fst
    |> expect
    |> toEqual("JRDOBMGGHMGNMVKG")
  );

  test("randomLength latin uppercase", () =>
    RandomString.randomLength(
      ~minLength=3,
      ~maxLength=8,
      RandomString.upperCaseLatinChar,
    )
    |> Generator.run(_, Seed.fromInt(28710))
    |> fst
    |> expect
    |> toEqual("XQOXMH")
  );

  test("randomLengthUpperCaseLatin", () =>
    RandomString.randomLengthUpperCaseLatin(~maxLength=8, ~minLength=3)
    |> Generator.run(_, Seed.fromInt(28710))
    |> fst
    |> expect
    |> toEqual("XQOXMH")
  );

  test("randomLengthLowerCaseLatin", () =>
    RandomString.randomLengthLowerCaseLatin(~maxLength=8, ~minLength=3)
    |> Generator.run(_, Seed.fromInt(-2879910))
    |> fst
    |> expect
    |> toEqual("psufj")
  );

  test("randomLengthLatin", () =>
    RandomString.randomLengthLatin(~maxLength=9, ~minLength=2)
    |> Generator.run(_, Seed.fromInt(-8154040))
    |> fst
    |> expect
    |> toEqual("Rdyw")
  );
});
