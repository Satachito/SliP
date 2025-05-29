git checkout firebase
git merge main

cp -r JS public/SliP
cp *.slip public/.
cp *.html public/.
cp *.png public/.
cp -r jp-split public/.
cp -r ReplaceNumeral public/.

firebase deploy

git checkout main
