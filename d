git checkout firebase
git merge main

cp -r SliP public/.
cp *.slip public/.
cp *.html public/.
cp *.png public/.
cp -r jp-split public/.

firebase deploy

git checkout main
