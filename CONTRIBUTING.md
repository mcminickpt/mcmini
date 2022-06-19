# Git Workflow

## Contributing to McMini 

We've chosen to follow the fork model
for contributing to the McMini repo.
Start by forking McMini on GitHub and 
clone down your new fork with

```
git clone my_repo.com ./my_repo
```

Then make sure you mark the main McMini repo
as a new remote with 

```
git remote add (main McMini repo name) (main McMini URL here)
```

You should then have two remotes: one for your
personal fork of McMini and another for the 
main McMini repo

One benefit of the fork model is that you
can customize your local branching workflow
for contributing to McMini; when you're ready
to add changes to the main repo, you can
create a PR targeting `main`

### The `main` branch

The `main` branch should contain the most stable version
of McMini. After cloning the repo and checking out the `main`
branch, users should be able to build and run the application
without any egregious bugs (e.g. immediate segfaults). 

Importantly, the `main` branch should *always* be free from compilation 
errors and should build without any issues. Any commit along the linear
history of the `main` branch should compile and run without issues.

### Local Branches

The contents of your local branches may begin to diverge from
`main` in the main repo. If your branches are not updated frequently enough, they
may become *stale* and it may be difficult to integrate changes introduced
in the branch. Thus, it is important to ensure that your local branches are
updated frequently both with new code added for the local branch and with
code incoming from other clones via PRs.

We follow the general rule of having no merge commits in the main repo whenever 
possible. This should result in a cleaner commit history and make tools
such as `git bisect` easier to use. Thus

**keep your local branches updated with `main` and update branches 
frequently, rebasing changes onto `main` in the main repo** 

### Naming Branches You Want Merged

To stay consistent, name local branches using *namespaces*. The highest
level namespace should be unique to you (e.g. using an abbreviation of 
your GitHub username or your name). Naming branches according to namespaces
ensures that it's clear who is the contributor of commits for the
branch in question. Further nesting is optional but may be helpful if you have
topic branches based off of other topic branches. For example, the branch

  `alice/bugs/fix-data-race`

could be used to describe a branch which is created by a user `alice`
for fixing bugs (in particular a data race)

## Commits

Commits are the primitives off of which the entire codebase is built upon.
Commits should ideally have the following characteristics:

  1. They should each be relatively *small* in their effect on the codebase.
  One commit should never introduce thousands of changes
  2. They should introduce specific changes. Avoid making commits such as "forgot
  to add a file" or "oops small fix" as these do not provide a clear explanation
  for why the commit was added in the first place, nor for what the commit actually
  introduces. Use `git merge --squash`, `git rebase -i`, or `git commit --amend` where 
  appropriate to reduce the number of vague commits

Commits should describe logical changes to the codebase. If the diff introduced
by a commit is not obvious, you should make sure to add comments to the 
commit message where appropriate (see the next section)

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

### Tags 

Tagging can be useful in cases where you want to mark milestones
represented by a series of committed changes. We haven't currently
agreed on a tagging system yet; but locally tagging your own commits
will help you identify the broader effect of your changes

## Issues

We should use issues to point out bugs, list features we want to merge into
mcmini in the future, and assign work. Issues should be named using the same
style as commit naming when appropriate (using the imperative etc.).

When you think of a new feature that should be added to McMini, make sure
to create an issue for it when appropriate. Use your judgement here: you probably
wouldn't, e.g., need to make an issue to rearrange code in a single file

### Labeling scheme

Whenever you make an issue, please make sure to tag it with the appropriate labels.
In particular, each issue should have one (or more) of each of the following tags:

	1. A `type` tag that identifies why the issue exists (e.g. to point out a bug)
	2. An `effort` tag describing how long the issue would take to be resolved'
	when efforts are begun to resolve it
	3. A `work` tag describing what the work might entail (known behavior
	or best practices)

## Pull Requests

Please make sure that you receive at least one approval
before merging in any changes to the main repository.
