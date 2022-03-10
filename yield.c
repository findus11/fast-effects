/*
-- an example using a relatively simple effects-based iterator/generator
-- interface. a real api based on this would most likely also define some
-- type `Iter(T) = () Unit / Yield(T)` s.t. functions like `mapped` below
-- could be used on arbitrary iterators, but that would abstract it a bit
-- too much for this example, i believe.
--
-- the gist of it is defining an effect `Yield(T)` (which in this case is
-- only used with `Nat`s or `unsigned`s in the c translation), and two
-- iterators: `mono(from Nat)`, which simply counts up from `from` ad
-- infinitum, and `mapped(from Nat, f (Nat) Nat)`, which calls `mono`
-- with `from` and calls `f` on all the values it yields. this is all
-- handled in `main`, which prints the values until it reaches one which
-- is larger than `100`. without the last condition, it will theoretically
-- run forever (since `mono` has no exit condition), assuming no crashes
-- on overflows.
--
-- the c code is a tad complicated in order to properly handle environments
-- (like the handler in `mapped` using the `f` defined outside its scope as
-- well as being effectful itself). if you've ever seen callbacks or closures
-- in c before, those shouldn't be too surprising, however.
type Yield(T)
eff yield[T](v T) Unit / Yield(T)

fun mono(from Nat) Never / Yield(Nat)
  var i = from
  loop
    yield(i)
    set i = i + 1
  end
end

fun mapped(from Nat, f (Nat) Nat) Never / Yield(Nat)
  mono(from) with Yield(Nat)
    yield(v) Unit
      yield(f(v))
    end
  end
end

fun sqr(x Nat) = x * x

fun main() Unit / Pure
  then mapped(0, sqr) with Yield(Int)
    yield(v) Unit
      println(v)
      if v >= 100 do
        break unit
      end
    end
  end
end
*/

#include <setjmp.h>
#include <stdio.h>

typedef void (*yield_t)(unsigned, jmp_buf *, void *);
struct eff_t {
  jmp_buf *env;
  yield_t yield;
  void *data;
};

static void mono(unsigned from, struct eff_t eff) {
  unsigned i = from;
  for (;;) {
    eff.yield(i, eff.env, eff.data);
    i = i + 1;
  }
}

typedef unsigned (*map_fn)(unsigned);
struct mapped_closure_data {
  struct eff_t eff;
  map_fn f;
};

static void yield_handler_mapped(unsigned v, jmp_buf *env, void *data) {
  (void)env;

  struct mapped_closure_data *mcd = (struct mapped_closure_data *)data;
  mcd->eff.yield(mcd->f(v), mcd->eff.env, mcd->eff.data);
}

static void mapped(unsigned from, map_fn f, struct eff_t eff) {
  jmp_buf env;
  int break_point;

  if (!(break_point = setjmp(env))) {
    struct mapped_closure_data data = {eff, f};
    struct eff_t mapped_eff = {&env, &yield_handler_mapped, &data};
    mono(from, mapped_eff);
  }
}

static void yield_handler_main(unsigned v, jmp_buf *env, void *data) {
  (void)data;

  printf("%u\n", v);
  if (v >= 100) {
    longjmp(*env, 1);
  }
}

static unsigned sqr(unsigned x) { return x * x; }

int main() {
  jmp_buf env;
  int break_point;

  if (!(break_point = setjmp(env))) {
    struct eff_t eff = {&env, &yield_handler_main, NULL};
    mapped(0, &sqr, eff);
  }

  return 0;
}
