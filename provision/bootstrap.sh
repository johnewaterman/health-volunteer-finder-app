#!/usr/bin/env bash
cp -r /vagrant/provision/etc/apt/* /etc/apt/
export DEBIAN_FRONTEND=noninteractive

apt-get update
apt-get -y upgrade
apt-get -y install \
  drush \
  git \
  libapache2-mod-php5 \
  mysql-server \
  php5-curl \
  php5-gd \
  php5-mysql \
  phpunit \
  rake \
  rsync \
  ruby-sass \
  ruby-compass \
  compass-susy-plugin \
  vim-nox

cp -r /vagrant/provision/etc/* /etc/

if ! pear list | grep PHP_CodeSniffer &> /dev/null ; then
	pear install PHP_CodeSniffer
fi

export DRAKE=/usr/local/share/drake
if [ ! -d $DRAKE ]; then
  git clone git@gitorious.org:drake/drake.git $DRAKE
fi

chmod -R u+w /vagrant/web/sites/default
cp /vagrant/provision/settings.php /vagrant/web/sites/default/

export FILES=/var/local/drupal
if [ ! -d $FILES ]; then
	mkdir -p $FILES
	chown -R www-data:staff $FILES
	chmod -R g+w $FILES
fi

if [ ! -L /vagrant/web/sites/default/files ]; then
	ln -s $FILES /vagrant/web/sites/default/files
fi

if [ ! -d /var/lib/mysql/drupal ]; then
	mysqladmin -u root create drupal
fi

if [ ! -L /etc/apache2/mods-enabled/rewrite.load ]; then
	a2enmod rewrite
fi

if [ -L /etc/apache2/sites-enabled/000-default ]; then
	a2dissite 000-default
fi

if [ ! -L /etc/apache2/sites-enabled/drupal ]; then
	a2ensite drupal
fi

cd /vagrant
rake -R $DRAKE db_sync_test_to_local
rake -R $DRAKE file_sync_test_to_local
service apache2 restart
