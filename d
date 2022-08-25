git checkout firebase
git merge main
cp *.js public/.
cp *.slip public/.
cp *.html public/.
cp *.png public/.
cp *.mp4 public/.
cp -r jp-split public/.
firebase deploy
git checkout main
