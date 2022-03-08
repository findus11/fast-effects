/*
-- decently simple proof-of-concept translation of the following.
--
-- from what i've seen, the "obvious" way of implementing effect
-- systems involves unwinding the stack, which, though i don't
-- know any actual performance characteristics, i've always
-- assumed was more heavyweight than necessary.
--
-- i've deliberately avoided reading up too much on other
-- implementations of effect systems, trying here and there to
-- develop my own ideas. the language here uses a somewhat simple
-- effect system, which notably doesn't support the idea of first-
-- class continuations. the major consequence of this is that
-- effects turn out to be essentially just dynamically scoped
-- functions with the ability to "break" out into their lexical
-- scope. combine this with a language which emphasises the
-- "function-like" part of effects and de-emphasizes the "break"
-- part, we can conjure up a system in which performing an effect
-- is in the common case as efficient as a closure call.
--
-- below is some pseudocode for a language which uses such a
-- system, followed by a simple-ish implementation of the same
-- program in c, where we've transformed the effects into closures
-- that can "break" using longjmp.
type E
eff e(x Int) Int / E

fun f(x Int) Int / E = e(x) + x
fun g(x Int) Int / E = f(x) + x

fun main() Unit / Pure
  let s = g(5) with E
    e(x) Int
      if x == 0 do
        break 123
      else
        return x + 1
      end
    end
  end

  then s.println()
end

-- g(5) = f(5) + 5 = e(5) + 5 + 5 = e(5) + 10 = 16
--> 16
*/

#include <setjmp.h>
#include <stdio.h>
#include <string.h>

typedef int (*e_t)(int, jmp_buf);
struct eff_t {
  jmp_buf env;
  e_t eff;
};

int f(int x, struct eff_t E) { return E.eff(x, E.env) + x; }
int g(int x, struct eff_t E) { return f(x, E) + x; }

int e_handler(int x, jmp_buf env) {
  if (x == 0) {
    longjmp(env, 123);
  } else {
    return x + 1;
  }
}

int main() {
  jmp_buf env;
  int break_point = setjmp(env);

  int s;
  if (break_point)
    s = break_point;
  else {
    // this is a not pretty very dirty hack because i'm too lazy to properly
    // figure out how actually initialize an array member. c really is funky.
    // sometimes, t a = x; is fine, but if t is actually a typedef'd array, then
    // that's not actually fine. i get why it is, since typedefs are pretty much
    // just fancy macros and arrays need initializers, but damn it is very
    // annoying.
    struct eff_t E = {.eff = &e_handler};
    memcpy(&E.env, &env, sizeof(env));
    s = g(5, E);
  }

  printf("%d\n", s);

  return 0;
}
