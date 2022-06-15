# Git Workflow

## Branch Organization

### The `main` branch

The `main` branch should contain the most stable version
of McMini. After cloning the repo and checking out the `main`
branch, users should be able to build and run the application
without any egregious bugs (e.g. immediate segfaults). 

Importantly, the `main` branch should *always* be free from compilation 
errors and should build without any issues. Any commit along the linear
history of the `main` branch should compile and run without issues.

We'll call a branch like main a *buildable* branch since it should always be
guaranteed to build and at least be semi-functional. 

### The `mcmini/unstable` branch

The `mcmini/unstable` branch contains more volatile code and includes
more recent (and hence unstable) changes to the codebase. Code
merged into `mcmini/unstable` should compile; that is, `mcmini/unstable`
should be buildable. Unlike `main`, however, there is less certainty
around `mcmini/unstable` in its functionality. There are more likely
to be bugs on `mcmini/unstable` than on the latest `main`.


### Topic branches

The most up-to-date changes will exist on the numerous topic branches,
each resulting in a new feature. Such branches should initially be based
off of `mcmini/unstable` or another topic branch based thereon; i.e.
`mcmini/unstable` should be the starting point from which new code changes
are made. Contributors should plan on breaking their work into

As a topic branch evolves, its contents may begin to diverge from
`mcmini/unstable`. If a topic branch is not updated frequently enough, it
may become *stale* and it may be difficult to integrate changes introduced
in the branch. Thus, it is important to ensure that a topic branch is
updated frequently both with new code added for the topic branch and with
code incoming from other topic branches into `mcmini/unstable`. We
follow the general rule of reducing the number of merge commits whenever 
possible. This should result in a cleaner commit history and make tools
such as `git bisect` easier to use. Thus

**keep topic branches updated with `mcmini/unstable` and update branches 
frequently, preferring to rebase changes onto `mcmini/unstable` over creating
merge commits**

Merge commits should ideally be reserved for pull requests. 

### Naming branches

To stay consistent, name topic branches using *namespaces*. The highest
level namespace should be unique to you (e.g. using an abbreviation of 
your GitHub username or your name). Naming branches according to namespaces
ensures that it's clear who is the main contributor of commits for the
branch in question. Further nesting is optional but may be helpful if you have
topic branches based off of other topic branches. For example, the branch

  `alice/bugs/fix-data-race`

could be used to describe a branch which is created by a user `alice`
for fixing bugs (in particular a data race)

#### Annotated tags

## Commits

Commits are the primitives off of which the entire codebase is built upon.
Commits should ideally have the following characteristics:

  1. They should each be relatively *small* in their effect on the codebase.
  One commit should ideally never introduce thousands of changes
  2. They should introduce specific changes. Avoid making commits such as "forgot
  to add a file" or "oops small fix" as these do not provide a clear explanation
  for why the commit was added in the first place, nor for what the commit actually
  introduces. Use `git merge --squash`, `git rebase -i`, or `git commit --amend` where 
  appropriate to reduce the number of vague commits


### Titling and commenting commits

Write commits using the imperative mood; for example, if a commit changes the behavior
of a certain class in the project, title it 

  "Change the behavior of ..."

instead of 

  "Change*d* the behavior of ..."

The idea is that commit titles tell you how the project should change when the commit is
introduced: "do" something  rather than "did" something.

For future maintainers of the codebase, it is also critically important to add a brief
description of the changes introduced in the commit as well as *why* those changes were
made. This is *especially* important for bug fixes. For bug fixes you should additionally
mention the root cause of the bug, if there were any alternative solutions you attempted
or considered and why or why not those failed, and how the changes in the commit (or commit
series following thereafter) squash the bug

FIXME: How should we outline the changes introduced in a commit?

## Issues

### Labeling scheme

## Pull Requests

When you've completed new changes to a pull request

### 

### 
