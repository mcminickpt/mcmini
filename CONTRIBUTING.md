# Git Workflow

## Contributing to McMini

Start by forking McMini on GitHub and clone down your new fork with

``` git clone <your-repo-url> ```

Then add the shared McMini repo as a new remote in your local checkout
with

``` git remote add <main McMini repo name> <main McMini URL> ```

You should then have two remotes: one for your personal fork of McMini
and another for the shared McMini repo

One benefit of the fork model is that you can customize your local
branching workflow for contributing to McMini; when you're ready to
add changes to the main repo, you can create a PR targeting the `main`
branch of the shared repo

### The `main` branch

The `main` branch should contain the most stable version of McMini.
After cloning the repo and checking out the `main` branch, users
should be able to build and run the application without any egregious
bugs (e.g. immediate segfaults)

Importantly, the `main` branch should *always* be free from
compilation errors and should build without any issues. Any commit
along the linear history of the `main` branch should compile and run
without issues.

**If you introduce commits in your local branches which do not
compile, be sure to prune those commits out or squash them into
commits that do build with `git rebase -i`, `git merge --squash`, or
`git commit --amend`!**

### Local Branches

The contents of your local branches may begin to diverge from `main`
in the main repo. If your branches are not updated frequently enough,
they may become *stale* and it may be difficult to integrate changes
introduced in the branch with new code added from other clones via
PRs.

The shared McMini repository is intended to have a linear commit
history.  This should result in a cleaner commit history and make
tools such as `git bisect` easier to use.

**Keep your local branches updated with `main` and update branches
frequently, rebasing changes onto `main` in the main repo**

## Commits

Commits should ideally have the following characteristics:

1. They should each be relatively *small* in their effect on the
codebase: one commit should never introduce thousands of changes

2. They should introduce specific changes. Avoid making commits such
as "forgot to add a file" or "oops small fix" as these do not
provide a clear explanation for why the commit was added in the first
place, nor for what the commit actually introduces. Use `git merge
--squash`, `git rebase -i`, or `git commit --amend` where appropriate
to remove such commits

Commits should describe logical changes to the codebase. If the diff
introduced by a commit is not obvious, you should make sure to add
comments to the commit message where appropriate (see the next
section). Better yet, you should split the commit, if possible, into
multiple smaller commits

### Moving Files

When you want to rename a file or move a file into another directory,
prefer using `git mv` over `mv` if the file is already under version
control. The former informs git that a file was simply moved or
renamed, and this is reflected in a commit's diff. If you use `mv`,
git will see the file as having been deleted and then re-created
inside another directory. This can be confusing and usually leads to a
larger diff than necessary

### Titling and commenting commits

Write commits using the imperative mood; for example, if a commit
changes the behavior of a certain class in the project, title it

  "Change the behavior of ..."

instead of

  "Change*d* the behavior of ..."

The idea is that commit titles tell you how the project should change
when the commit is introduced: "do" something  rather than "did"
something.

For future maintainers of the codebase (including yourself!), it is
also critically important to add a brief description of the changes
introduced in the commit as well as *why* those changes were made.
This is *especially* important for bug fixes. For bug fixes you should
additionally mention the root cause of the bug, if there were any
alternative solutions you attempted or considered and why or why not
those failed, and how the changes in the commit (or commit series
following thereafter) fix the bug

## Issues

Issues point out bugs and document features we want to merge into
McMini in the future. Issues should be named using the same style as
commit naming when appropriate (using the imperative etc.).

When you think of a new feature that should be added to McMini, make
sure to create an issue for it when appropriate. Use your judgement
here: you probably wouldn't need to make an issue to rearrange code in
a single file

### Labeling scheme

Whenever you make an issue, make sure to tag it with the appropriate
labels.  In particular, each issue should have one (or more) of each
of the following tags:

  1. A `type` tag that identifies why the issue exists (e.g. to point
     out a bug)
  2. An `effort` tag describing how long the issue would take to be
     resolved' when efforts are begun to resolve it
  3. A `work` tag describing what the work might entail (known
     behavior	or best practices)
