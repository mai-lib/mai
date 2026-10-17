"""mkdocs hook: point links that leave docs/ at the files on GitHub.

The markdown in docs/ is written to read well on GitHub, so it links to
../examples and ../bench directly. Those directories are not part of the
built site, so on the site the links go to the repository instead.
"""

import re

REPO_TREE = "https://github.com/mai-lib/mai/tree/main/"
REPO_BLOB = "https://github.com/mai-lib/mai/blob/main/"

_LINK = re.compile(r"\]\(\.\./([^)\s]+)\)")


def _rewrite(match):
    path = match.group(1)
    base = REPO_BLOB if "." in path.rsplit("/", 1)[-1] else REPO_TREE
    return "](" + base + path + ")"


def on_page_markdown(markdown, **kwargs):
    return _LINK.sub(_rewrite, markdown)
