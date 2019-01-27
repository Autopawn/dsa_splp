fnames=$(find splp/ | grep -v '.opt' | grep -v '.bub' | grep -v 'README' | grep -v '.c' | grep -v '.lst' | grep -v '~')

parameters="n_100_200 n_300_600 n_300_-1 h_300_600"

make

rm -rf res || true

for params in $parameters; do
    problems=""
    for p in $fnames; do
        if [[ -d $p ]]; then
            if [ -n "$problems" ]; then
                gname="$(echo $group | tr / _ | cut -d'_' -f2-)"
                name="$params"_"$gname"
                qsub -N $name solve.sh \
                    -F "$params \"$group\" \"$problems\"" || \
                bash solve.sh $params "$group" "$problems"
            fi
            problems=""
            group="$p"
        elif [[ -f $p ]]; then
            problems="$p $problems"
        else
            echo "$p is not valid"
            exit 1
        fi
    done
done
