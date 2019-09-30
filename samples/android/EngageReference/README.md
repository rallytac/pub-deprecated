# EngageReference
The Engage reference application consists of 3 components, the Engine, the Core, and the UI.  The Engine is an Android AAR archive that contains the Engage Engine binary distributed in .so format (it's a compiled C++ library built with the Android NDK).  

The Core is an AAR which uses the Engine AAR and functions as a container for the C++ library, and intermediary for the UI to communicate with the Engine, and a set of user interface elements that are pretty standard across Engage applications (such as Settings, QR code functionality, Google Mapping, and so forth).

The UI is an Android app which consists solely of the main activity that a user works with.  For this project, we've provided a Star Trek theme UI which pays homage to the Star Trek "LCARS" user interface which we all know and love.  (Check out https://en.wikipedia.org/wiki/LCARS for more information.) The APK that comes from this app will contain the Engine and Core AARs as well as, of course, the UI.

There's a few things you need to be aware of, though, in using these modules.

1) The Core, upon firing up and making sure that all is good, needs to launch an activity to be shown to the user.  As this activity is ostensibly outside the Core AAR, the launcher (LauncherActivity.java) needs to be told which activity this is.  You do this by specifying the name of that activity in the UI's Android manifest file as a meta-data element.

2) If you want to use Google Maps in your UI, you will need to get an API key from Google.  Once you have that, place it in the "com.google.android.geo.API_KEY" meta-data element in your UI's Android manifest file.

Live Long And Prosper!
