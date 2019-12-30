open Relude.Globals;

// Bitwise operations are already globally available via OCaml's Pervasives, but
// I find it challenging to keep track of infix `lsl`, so I'm aliasing them
// here, which also makes it slightly easier to translate from Elm's named
// Bitwise operations.
module Bitwise = {
  // let and_ = (a, b) => a land b;
  // let or_ = (a, b) => a lor b;
  // let xor = (a, b) => a lxor b;
  let shiftRightZeroFill = (a, b) => b lsr a;
};

type t =
  | Seed(int, int);

// produce the next seed, given a current seed
let next = (Seed(state0, inc)) => Seed((state0 * 1664525 + inc) lsr 0, inc);

// produce a pseudorandom int from a seed
let peel = (Seed(state, _)) => {
  let vshift = Bitwise.shiftRightZeroFill(state lsr 28 + 4, state);
  let word = state lxor vshift * 277803737;
  let shift22 = Bitwise.shiftRightZeroFill(22, word);
  Bitwise.shiftRightZeroFill(0, shift22 lxor word);
};

let fromInt = x => {
  let Seed(state1, inc) = next(Seed(0, 1013904223));
  let state2 = (state1 + x) lsr 0;
  next(Seed(state2, inc));
};

let init: IO.t(t, Void.t) =
  IO.suspendWithVoid(() => Js.Date.now())
  |> IO.map(ms => fromInt(int_of_float(ms)));
