#!/usr/bin/python2

# Override an object's method with a new function, and optionally
# install/alter specified object variables. The original and new function
# must accept the same arguments. The old function will be available as obj.__override__
def override(objfunc, newfunc, **opts):
    obj=objfunc.im_self
    if not hasattr(obj, '__override__'):
        class container: pass
        setattr(obj, '__override__', container())
    if objfunc.__name__ in obj.__override__.__dict__: raise Exception(str(obj.__override__.__dict__))
    obj.__override__.__dict__[objfunc.__name__]=objfunc.__get__(obj, obj.__class__)
    setattr(obj, objfunc.__name__, newfunc.__get__(obj, obj.__class__))
    if opts: vars(obj).update(opts)

class test(object):
    def __init__(self, name):
        self.name = name

    def show(self):
        print self.name
        print

a=test("I am A")
b=test("I am B")
c=test("I am C")

a.show()
b.show()
c.show()

# this function can replace the original object's show method
def newshow(self):
    print "Hello from number %d" % self.number
    # invoke the original show method
    self.__override__.show()

override(a.show, newshow, number=1)
override(b.show, newshow, name="I am still B", number=2)

a.show()
b.show()
c.show()
