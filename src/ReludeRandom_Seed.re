open Relude.Globals;

type t =
  | Seed(int, int);

// produce the next seed, given a current seed
let next = (Seed(state0, inc)) => Seed((state0 * 1664525 + inc) lsr 0, inc);

// produce a pseudorandom int from a seed
let peel = (Seed(state, _)) => {
  let vshift = state lsr (state lsr 28 + 4);
  let word = state lxor vshift * 277803737;
  let shift22 = word lsr 22;
  (shift22 lxor word) lsr 0;
};

let fromInt = x => {
  let Seed(state1, inc) = next(Seed(0, 1013904223));
  let state2 = (state1 + x) lsr 0;
  next(Seed(state2, inc));
};

let init: IO.t(t, Void.t) =
  IO.suspendWithVoid(() => Js.Date.now())
  |> IO.map(ms => fromInt(int_of_float(ms)));
