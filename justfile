alias u := update
alias p := push
alias s := sync

update msg:
    git add .
    git commit -m "{{msg}}"

push:
    git push

sync:
    git submodule update --remote --merge
