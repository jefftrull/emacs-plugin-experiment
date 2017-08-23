// A simple C++ plugin for Emacs
#include <iostream>
#include <cassert>

#include <emacs-module.h>

// signal to Emacs that we are libre
extern void *plugin_is_GPL_compatible;

emacs_env * eenv;   // for use by our code

// things we can do with eenv:
// create/free globals
// force function exit
// register and call functions
// get the type of a value and test it for nil, equality
// extract/create ints and floats
// insert strings into buffers
// create/get/set user_ptr (custom C/C++ types embedded for us)
// register "finalizer" (dtor) for user_ptr
// get, set, and determine size of Emacs "vector"

// Very rudimentary binding example, for now:
// We create a Lisp function taking no arguments and returning nil
// We call it from within C++, then give it a name so you can call it from Emacs
// You can test all of this with a one-liner:
// emacs -l ./libdymod.so -f dymod-sample-nullary-void-fn
// it will print "Hello Elisp" twice on the console

// Our API
void nullary_void_fn() { std::cout << "Hello Elisp\n"; }

int emacs_module_init(struct emacs_runtime *ert) {
    // get the current environment
    eenv = ert->get_environment(ert);

    // register a some simple functions
    // create the Emacs-side function wrapper object
    emacs_value fn = eenv->make_function(eenv, 0, 0,
                        [](emacs_env *env, ptrdiff_t, emacs_value [], void*) {
                            nullary_void_fn();
                            return env->intern(env, "nil");
                        },
                        "An example nullary void function bound from C++",
                        nullptr);  // a way to supply associated data in the callback

    // we can call it right away if we want to
    emacs_value retval = eenv->funcall(eenv, fn, 0, nullptr);

    // the result should be nil
    assert(!eenv->is_not_nil(eenv, retval));
    (void)retval;   // avoid unused variable error in Release builds

    // we can also bind it to a symbol a la defun:

    // first, create a symbol we want to associate with the function
    emacs_value fname = eenv->intern(eenv, "dymod-sample-nullary-void-fn");

    // next get the fset function from Emacs
    emacs_value fset = eenv->intern(eenv, "fset");

    // call fset
    emacs_value args[]{fname, fn};
    eenv->funcall(eenv, fset, 2, args);

    // now we can call our named function from within Emacs

    return 0;
}
void *plugin_is_GPL_compatible = nullptr;

struct SomeClass {
    SomeClass() {}
    ~SomeClass() {}
};

// happens via ld, prior to emacs_module_init
// will be destroyed when Emacs exits
static SomeClass foo;
