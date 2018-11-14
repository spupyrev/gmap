GMap: Graph-to-Map
================
GMap is a system for visualizing graphs as maps.

Basic Setup
--------

1. Install the python dependencies listed in [requirements.txt](requirements.txt). Using pip:

        pip install -r requirements.txt

2. Install [graphviz](http://graphviz.org/Download..php)

3. Compile the extranal libraries by running (optional)

        make -C ./external/eba
        make -C ./external/ecba
        make -C ./external/mapsets

4. Set up Django settings (optional).
Edit `DATABASES`, `SECRET_KEY`, `ALLOWED_HOSTS` and `ADMINS` in `gmap_web/settings.py`

5. Create Django databases:

        ./manage.py syncdb

6. Run the server:

        ./manage.py runserver
  
7. Access the map interface at `http://localhost:8000`

License
--------
Code is released under the [MIT License](MIT-LICENSE.txt).
