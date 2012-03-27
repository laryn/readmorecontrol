Read More Control

This module adds settings to node content types, and node field instances to 
control when the 'Read More' link is displayed in a teaser.

When editing a content type a new setting 'Display Read More' will be added
under a new 'Read More Settings' tab. This setting can be set to one of
       Always - The 'Read More' link is always shown in Teaser View. This is
                the way unextended Drupal 7 acts and is the default
When Required - The 'Read More' link will only be shown if the displayed 
                content in teaser view differs from full view
        Never - The 'Read More' link will never be displayed.
		
When editing a field in a content type a new setting 'Read More Ignores Field'
is added to the per type settings. When set, the field will not be checked when
the content type has the 'Display Read More' setting set to 'When Required'.
It will be assumed that the field is the same in full and teaser views, even if
they are not. This can be very useful.

An example of when 'Read More Ignores Field' can be useful is if your teaser 
displays an image thumbnail that links to the full article that displays the
full sized image in it. If there is body text and the display between full and
teaser views don't differ, then the read more link wouldn't be displayed. To 
see the full image a visiter can just click on the image. However if the 
body text does differ a 'Read More' link would be displayed indicating there is
more body text in the article.

Another example when 'Read More Ignores Field' can be useful is to stop the 
'Read More' links from appearing if want to hide taxonomy fields from teaser 
view. Normally when the 'Display Read More' setting is set to 'When Required'
if you have a hidden field in teaser view that is not hidden in full view, the
'Read More' link will be shown. By setting the 'Read More Ignores Field' 
setting on something like a Taxonomy field you can stop it from being shown in 
the teaser but can still have it shown in the full view and not have a 
'Read More' link shown. 

As a note, even if the 'Read More' link is not shown, clicking on the Title of 
a node teaser will show the full node, so full node view is still easy to see.
