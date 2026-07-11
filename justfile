alias u := update
alias p := push
alias s := sync
alias ds := deinit-submodule

update msg:
    git add .
    git commit -m "{{msg}}"

push:
    git push

sync:
    git submodule update --remote --merge

bootstrap:
    git submodule update --init --recursive
    ./scripts/bootstrap.sh

deinit-submodule path:
    git submodule deinit -f -- {{ path }}
    rm -rf .git/modules/{{ path }}
    git rm -f {{ path }}
