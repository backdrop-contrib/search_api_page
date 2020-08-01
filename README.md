Search API Pages
----------------

This module allows you to create simple search pages and search blocks for
searching in your Search API indexes, similar to the core Search functionality.

Installation
------------


- Install this module using the official Backdrop CMS instructions at
  https://backdropcms.org/guide/modules.

- As the only pre-requisite you will need the Search API [1] module enabled and
correctly set up, as well as the Entity Plus [2] module enabled, along with at
least one service class. Create a server and an index (or use the default one)
and configure them according to your needs.

[1] https://github.com/backdrop-contrib/search_api
[2] https://github.com/backdrop-contrib/entity_plus

- Go to admin/config/search/search_api/page on your site (Administration »
Configuration » Search and metadata » Search API » Search pages) where you can
add a search page for your index.

Documentation
-------------

Additional documentation is located here:
https://www.drupal.org/node/1255226

Issues
------

- Pager not displayed
  It can sometimes happen in specific setups, that even though your search
  returns more results than appear on the page, no pager is displayed for
  browsing the other results. This will most likely be due to a second pager
  being displayed somewhere on the page, which Drupal cannot handle without some
  additional configuration. To fix this problem, locate the other pager (the one
  being displayed) on the page and attempt to change the pager element it uses.
  E.g., for Views you can change the element by going to the configuration of
  the view in question, open the pager settings and set the "Pager ID" to 1 or
  greater. This should fix the problems with the pager.
  See [2] for details, and for help if the other pager doesn't come from Views.

  [2] https://drupal.org/node/1442686
  
- Bugs and Feature requests should be reported in the Issue Queue:
https://github.com/backdrop-contrib/search_api_pages/issues.

Current Maintainers
-------------------

- [Ray Burgess](https://github.com/earlyburg).

Credits
-------

- Ported to Backdrop CMS by [Ray Burgess](https://github.com/earlyburg).
- Originally written for Drupal by [Thomas Seidl](https://www.drupal.org/u/drunken-monkey).

License
-------

This project is GPL v2 software. 
See the LICENSE.txt file in this directory for complete text.

---------------


