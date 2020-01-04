let char = (~minCode, ~maxCode) =>
  ReludeRandom_RandomInt.make(~min=minCode, ~max=maxCode)
  |> ReludeRandom_Generator.map(Js.String.fromCharCode);

let upperCaseLatinChar = char(~minCode=65, ~maxCode=90);

let lowerCaseLatinChar = char(~minCode=97, ~maxCode=122);

let latinChar =
  ReludeRandom_Generator.(
    uniform(lowerCaseLatinChar, [upperCaseLatinChar]) |> flatten
  );

let englishChar = latinChar;

let asciiChar = char(~minCode=0, ~maxCode=127);

let basicLatinChar = asciiChar;

let latin1SupplementChar = char(~minCode=128, ~maxCode=255);

let latinExtendedAChar = char(~minCode=256, ~maxCode=383);

let latinExtendedBChar = char(~minCode=384, ~maxCode=591);

let make = (~length, charGen) =>
  ReludeRandom_RandomList.make(~length, charGen)
  |> ReludeRandom_Generator.map(Relude.List.String.join);

let makeLatin = (~length) => make(~length, latinChar);

let makeLowerCaseLatin = (~length) => make(~length, lowerCaseLatinChar);

let makeUpperCaseLatin = (~length) => make(~length, upperCaseLatinChar);

let randomLength = (~minLength, ~maxLength, charGen) =>
  ReludeRandom_RandomInt.make(~min=minLength, ~max=maxLength)
  |> ReludeRandom_Generator.flatMap(length => make(~length, charGen));

let randomLengthLatin = (~minLength, ~maxLength) =>
  randomLength(~minLength, ~maxLength, latinChar);

let randomLengthLowerCaseLatin = (~minLength, ~maxLength) =>
  randomLength(~minLength, ~maxLength, lowerCaseLatinChar);

let randomLengthUpperCaseLatin = (~minLength, ~maxLength) =>
  randomLength(~minLength, ~maxLength, upperCaseLatinChar);
