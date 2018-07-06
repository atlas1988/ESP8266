sudo apt-get update
sudo apt-get -y install  git ssh vim
#sudo apt-get install bison
#sudo apt-get install gperf
#sudo apt-get install flex
#sudo apt-get install libncurses5-dev
#sudo apt install python-pip
#sudo pip install pyserial 

git config --global user.name "atlas1988"
git config --global  user.email "atlas_li@126.com"
git config --global commit.template .git_template
git config --global core.editor vim
git config --global color.ui auto

ssh-keygen -t rsa
sudo cp ~/.ssh/id_rsa.pub id_rsa.pub
vi id_rsa.pub

