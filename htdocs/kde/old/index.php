<?php include 'header.php'; ?>

<p>Return to <a href="../index.php">SynCE main page</a>.</p>
<h1>SynCE - KDE Integration</h1>
<p>In this text the term "PDA"&nbsp; is used as a synonym for all
devices running Windows CE Version 2.0 or later as long as they
are supported by the underlying <a
 href="http://synce.sourceforge.net/synce/">SynCE libraries</a>.</p>
<table border="0" cellpadding="2" cellspacing="2" width="100%">
  <tbody>
    <tr>
      <td valign="top" width="50%">
      <h2><a name="Contents" id="Contents"></a>Contents</h2>
      <ul>
        <li><a href="#Introduction">Introduction</a></li>
        <li><a href="#News">News</a></li>
        <li><a href="#Features">Features</a></li>
        <li><a href="#Usage">Usage</a><br></li>
        <li><a href="#Requirements">Requirements</a></li>
        <li><a href="#Download">Download</a><br></li>
        <li><a href="#Installation">Compiling and Installing</a></li>
        <li><a href="screenshots.php">Screenshots</a></li>
        <li><a href="#Related_Work">Related Work</a></li>
        <li><a href="stories.php">Success Stories</a></li>
        <li><a href="faq.php">Frequently asked Questions</a></li>
        <li><a href="#Future">Future</a></li>
      </ul>
      </td>
      <td valign="top" width="50%">
      <h2><a name="Synchronization" id="Synchronization"></a>Synchronization</h2>
      <ul>
	<li><a href="konnector/index.php">KitchenSync Konnector</a></li>
      </ul>
      <h2><a name="Add-on" id="Add-on"></a>Add-on</h2>
      <ul>
        <li><a href="kcemirror.php">KCEMirror</a></li>
      </ul>
      <h2><a name="Links" id="Links"></a>Links</h2>
      <ul>
        <li><a href="http://sourceforge.net/projects/synce/">SourceForge
Project Page</a></li>
        <li><a
 href="http://www.kde-apps.org/content/show.php?content=14162">SynCE-KDE
on www.kde-apps.org</a> </li>
        <li><a
 href="http://www.kde-apps.org/content/show.php?content=14276">SynCE-KCEMirror
on
www.kde-apps.org</a></li>
        <li><a
 href="http://freshmeat.net/projects/synce-kde/?topic_id=20%2C957%2C1015">SynCE-KDE
on freshmeat.net </a></li>
      </ul>
      </td>
    </tr>
  </tbody>
</table>
<br>
<h2><a name="News" id="News"></a>News</h2>
<b>September 4, 2004</b> <a href="mailto:jmcc_dot_stein_at_web_dot_de">
Carsten Stein</a> has contributed a rpm-package designed for SuSE-9.1. 
You could download this package from the 
<a
href="http://sourceforge.net/project/showfiles.php?group_id=30550&package_id=73834&release_id=264057">
usual place at sourceforge</a>. - Thank you Carsten!<br><br>
<b>August 30, 2004</b> SynCE-KDE 0.8.0 released! This new release is
known to work with version 0.9.0 of the SynCE libraries and KDE-3.1,
KDE-3.2, or KDE-3.3.<br>
<blockquote><b>Changes</b><br>
  <ul>
    <li>Partnership management completely new implemented.</li>
    <li>Behavior of buttons in dialogs more intuitively.</li>
    <li>AvantGo-Client install wizzard added (automatic install-mode
removed).</li>
    <li>KDE-Addressbook Synchronizer has been removed (will be replaced
by a
kitchensync-based solution soon).</li>
    <li>Code prepared for localization.</li>
    <li>First version of a german translation added. </li>
    <li>Many small bugs fixed.</li>
  </ul>
  <b>Some notes for upgrade from a previous release</b><br>
  <blockquote>In 0.8.0 the KDE-Addressbook Synchronizer has been
removed because it was depreciated since version 0.7. If you are
compiling SynCE-KDE by yourself, the libraries and the service-control
files of the KDE-Addressbook <br>
Synchronizer are not removed automatically. Please remove the following
files from your harddisk by hand:<br>
    <ul>
      <li>librakikdeaddressbooksync.* usually located in /usr/lib,
/opt/kde/usr/lib, or ...</li>
      <li>rakikdeaddressbooksync.desktopusually located in
/usr/share/services, /opt/kde/usr/share/services, or ...</li>
    </ul>
If you could not find those files in one of those directories try to
find it <br>
by use of the "locate" and/or "find" commandline tools.<br>
  </blockquote>
</blockquote>
<b>July 08, 2004</b> It has been long time ago since the last entry
here in this News
section. There has not been that much progress in the last view month.
Nevertheless, some points should be mentioned:
<ul>
  <li>Lets welcome Christinan Fremgen, a new developer for the "big"
SynCE-KDE team
:-) He is mainly commited in the field of KDE - Windows CE
synchronization and
is actually bussy in designing and implementing KAddressBook, and
KOrganizer
synchronization by use of the KDE synchronization framefork kitchensync.<br>
Unfortunately, the old synchronization mechanism for KAddressBook
doesn't
reliable work any longer. We have to be patient until we get the new
synchronization plugins ready. We expect a pre-rease to be available in
september
2004.</li>
  <li>RAKI and also RAPIP has been ported to KDE verion 3.2.x. </li>
  <li>AvantGo-Synchronization has been much improved. The whole
configuration
could now be done in RAKI itself.</li>
  <li>PDAMirror has been renamed to KCEMirror as this name integrates
nicely into
the KDE name-schema.</li>
  <li>SynCE-KDE and most of the SynCE libraries and tools are now part
of debian
unstable. A list of those packages could be find <a
 href="http://qa.debian.org/developer.php?login=Volker+Christian">here</a>.</li>
</ul>
<b>November 21, 2003</b> PDAMirror is integrated into the SynCE-KDE
suite<br>
<br>
<!--
<b>October 9, 2003</b> There is some progress in letting SynCE-KDE
(cvs) run on
KDE-3.1.90 (cvs) the upcoming KDE-3.2 release.
<ul>
  <li>RAPIP is running. There are of course some things left to be
worked on.</li>
  <li>RAKI is also working but the KDE addressbook synchronizer is not
due to massive
changes in the KABC API. Nevertheless, the synchronizer would be
compiled, but do
not use it - RAKI would be blocked entireyl.</li>
  <li>VDCCM doesn't have any problems on KDE-3.1.90 (cvs) at all.</li>
</ul>
<b>September 9, 2003</b> SynCE-KDE 0.6.1 is released!<br>
<ul>
  <li>This is a bug fix release. The annoying "33%" bug which has
prevented RAKI to run on some Linux distributions is fixed.</li>
  <li>Some unnecessary calls into librra has been removed. RAKI should
be a little faster now.</li>
  <li>Debian packages could now be installed directly over the network.
Have a look at the <a href="#Download">download</a> section for
informations about that.<br>
  </li>
</ul>
<b>August 18, 2003</b> SynCE-KDE 0.6&nbsp; is released!<br>
<ul>
  <li>First of all the name of the project has changed. It is now
called SynCE-KDE.</li>
  <li>Secondly - yes - the release number jumps directly from 0.4
to 0.6. Release 0.5 was an internal release never published for
public use.</li>
  <li>The MULTIPLE_DEVICES branch is entirely merged into HEAD of <a
 href="http://sourceforge.net/cvs/?group_id=30550">CVS</a>. This
release is a snapshot of it. You can download <tt>tgz</tt>, <tt>rpm</tt>,
and <tt>deb</tt> packages from the <a
 href="http://sourceforge.net/projects/synce/">SourceForge Project
Page</a>. Have a look at the <a href="#Features">features</a>
section for a list of all goodies.</li>
  <li>RAPIP is considered to be stable and full functional.
Nevertheless, there are some additional features and improvements
planed in the future.</li>
  <li>RAKI comes with a new plugin-architecture. Plugins, called
synchronizer, are used to synchronize PDAs with your desktop
system.</li>
  <li>An alternative DCCM called VDCCM is also included in the new
release. You have to use this alternative DCCM if you intent to
connect more than one PDA at the same time to your desktop.</li>
  <li>Version 0.6 should be seen as a "Vertical Slice" and a "Proof
of Concept". Almost all planed features are implemented. Of course
there are rough edges and odd bugs. Some code is also still
missing. At the moment there are only two synchronization plugins
(synchronizer) available. Help, especially in this area is badly
needed. If you are interested in helping, please send mail to the <a
 href="http://sourceforge.net/mail/?group_id=30550">development
mailing list.</a></li>
</ul>
<br>
-->
<h2><a name="Introduction" id="Introduction"></a>Introduction</h2>
<p>One effort in integrating <a
 href="http://synce.sourceforge.net/synce">SynCE</a> into <a
 href="http://www.kde.org">KDE</a> is the SynCE-KDE project - formerly
Rapip/Raki. SynCE-KDE essentially consists of the three
subproject <b>RAPIP,</b> <b>RAKI,</b> and <b>VDCCM</b>. In
general, they all support the same devices which are supported
by the underlying SynCE libraries.</p>
<p><b>RAPIP</b> <br>
</p>
<blockquote>is the short form for "<b>RAPI</b>-<b>P</b>rotocol".
It is a full featured KDE io-slave used to browse through the PDA
file system and to copy files to and from the PDA by drag and drop
via <a href="http://konqueror.kde.org">Konqueror</a>.</blockquote>
<p><b>RAKI</b> <br>
</p>
<blockquote>is a synonym for
"<b>R</b>emote-<b>A</b>ccess-<b>Ki</b>cker-applet". As the name
suggests it has first been an applet for kicker, the KDE panel but
as time goes by the application-type changed. Today RAKI&nbsp; is
an application docking into the KDE system-tray. It is responsible
for all additional tasks which are desired for interacting with the
PDA via the desktop.</blockquote>
<p><b>VDCCM</b> <br>
</p>
<blockquote>is a generalization of the native <a
 href="http://synce.sourceforge.net/synce/architecture.php#dccm">SynCE-DCCM</a>
but is fully compatible with it. In addition to all DCCM features
it is capable of supporting more than one PDA connected at the same
time. It is responsible for reporting all connects and disconnects
of PDAs to interested clients via an unix-socket by use of a simple
protocol. E.g. RAKI is one of such an interested client.<br>
  <br>
</blockquote>
<h2><a name="Features" id="Features"></a>Features</h2>
<b>RAPIP</b>
<ul>
  <li>copies files and directories via drag and drop to and from the
PDA.</li>
  <li>is capable of handling mime-types.</li>
  <li>supports more than one connected device simultaneously.</li>
</ul>
<br>
<b>RAKI</b>
<ul>
  <li>also supports more than one connected PDA at once.<br>
  </li>
  <li>knows how to handle PDA partnerships.<br>
  </li>
  <li>uses a plugin-architecture for synchronization. Two
synchronizer-plugins are already implemented:<br>
    <ul>
      <li>Synchronizer for PDA-Contacts and standard KDE address book
synchronization.<br>
      </li>
      <li><a href="http://www.avantgo.com/">AvantGo</a> synchronizer.</li>
    </ul>
  </li>
  <li>creates a NAT-Route for your PDAs to the outer internet.</li>
  <li>supports configuration of passwords, NAT routes, and
synchronization settings per device.<br>
  </li>
  <li>installs cab-files on the device - you just have to drag them
over the system-tray or right-click on a <tt>cab</tt> file in
Konqueror.</li>
  <li>lets you manage the software installed on the PDA.</li>
  <li>displays system information and power status about the
PDA.</li>
  <li>starts programs on the PDA.</li>
  <li>answers password-requests of PDAs automatically.</li>
  <li>notifies about connections, password requests and
disconnections of PDAs with sound.</li>
</ul>
<br>
<b>VDCCM</b>
<ul>
  <li>is responsible that RAPIP and RAKI can communicate with more
than one PDA simultaneously.</li>
  <li>creates an individual "active_connection" file for every
connected PDA.</li>
  <li>the names of individual "active_connection" files are either
the PDA identifiers or the ip-addresses assigned to the PDAs.</li>
</ul>
<br>
<h2><a name="Usage" id="Usage"></a>Usage</h2>
<b>RAPIP</b>
<dl>
  <dd>Open Konqueror and type <tt>rapip://&lt;name_of_an_active_connection_file&gt;/</tt>
into
the URL line of Konqueror. You should see the root-directory of the
PDA which corresponds to the specified active_connection file. Use
Konqueror as usual - e.g. drag and drop files to and from the PDA
file system ...<br>
If there is just one PDA connected, you can also use the short
form <tt>rapip:/</tt> to browse this PDA.<br>
  </dd>
</dl>
<br>
<b>RAKI</b>
<dl>
  <dd>Launch the application from the "K-Menu-&gt;Utilities"-Menu
or,&nbsp; if you don't have a "K-Menu-&gt;Utilities-&gt;Raki" entry,
start RAKI from the
command line by issuing "raki". You will get a new icon located in the
system-tray.<br>
Right click on that icon shows a popup menu containing
infrastructure related items.<br>
If there are connected devices left click will open a popup menu
containing individual device menu items for every connected PDA.<br>
Drag and drop of CAB-files onto the RAKI system-tray icon installs
them
on a device. A dialog box lets you choose the destination
PDA.<br>
Choosing "Synchronize" from an individual device menu starts
synchronizing that PDA and your desktop.<br>
  </dd>
</dl>
<br>
<b>VDCCM</b>
<dl>
  <dd>RAKI can launch VDCCM at startup (see the "Configure" menu
entry in the infrastructure menu) - that's the common case.<br>
Nevertheless, VDCCM supports a view command line arguments in
addition to the usual DCCM command line arguments. Issue <tt>vdccm
-h</tt> for a list of them. You can start VDCCM completely
independent of RAKI by hand before or after RAKI has been started.
Of course, no (V)DCCM may be running at this time.</dd>
</dl>
<br>
<h2><a name="Requirements" id="Requirements"></a>Requirements</h2>
<p>SynCE-KDE 0.6 requires</p>
<ul>
  <li><a href="http://www.kde.org">KDE 3.1</a> or later,<br>
  </li>
  <li><a href="http://www.trolltech.com/products/qt/index.html">Qt
3.1</a> or later, and of course</li>
  <li>the <a
 href="http://sourceforge.net/project/showfiles.php?group_id=30550&amp;release_id=142059">
SynCE libraries</a> version 0.9.</li>
</ul>
<br>
<h2><a name="Download" id="Download"></a>Download</h2>
<br>
<span style="font-weight: bold;"><a href="http://www.debian.org/">Debian</a>
user</span><br>
<blockquote>SynCE-KDE is now part of debian.</blockquote>
<a href="http://sourceforge.net/cvs/?group_id=30550"><b>CVS</b></a><b>
user</b><br>
<blockquote>If you are interested in testing bleeding edge software you
could
also check out the latest code directly from <a
 href="http://sourceforge.net/cvs/?group_id=30550">CVS</a> by issuing
the
command<code></code><br>
  <br>
  <code>&nbsp;&nbsp;&nbsp; # cvs -z3
-d:pserver:anonymous@cvs.sourceforge.net:/cvsroot/synce \<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; co synce-kde</code><br>
  <blockquote><code></code></blockquote>
  <code></code>after <a
 href="http://sourceforge.net/cvs/?group_id=30550">logging
in</a>. Look at <a href="#Installation">Compiling and Installing</a>
for a guide how to compile and install SynCE-KDE by yourself.<br>
</blockquote>
<b>Source Code</b><br>
<blockquote>Download the source code of SynCE-KDE from the <a
 href="http://sourceforge.net/projects/synce/">SourceForge
Project Page</a>.<br>
</blockquote>
<a href="#"> </a><br>
<h2><a name="Installation"
 href="http://sourceforge.net/cvs/?group_id=30550" id="Installation"></a>Compiling
and Installing</h2>
<h3>Requisites</h3>
<ul>
  <li>Install the base SynCE libraries as described <a
 href="http://synce.sourceforge.net/synce/using.php#download">here</a>.</li>
  <li>Install the following additional libraries<br>
  </li>
  <ul>
    <li><a href="http://synce.sourceforge.net/synce/dynamite.php">libdynamite</a></li>
    <li><a href="http://synce.sourceforge.net/synce/unshield.php">libunshield</a></li>
    <li><a href="http://synce.sourceforge.net/synce/orange.php">liborange</a><br>
    </li>
  </ul>
  <li>Make the SynCE libraries work by <a
 href="http://synce.sourceforge.net/synce/using.php#gettingconnected">getting
connected</a> for the first time.<br>
  </li>
  <li>If you want the <a href="http://www.avantgo.com/">AvantGo</a>
synchronizer plugin to be compiled, you
also have to download and compile <a
 href="http://www.mechlord.ca/%7Elownewulf/agsync-0.2-pre.tgz">agsync-0.2-pre.tgz</a>
from&nbsp; <a href="http://www.mechlord.ca/%7Elownewulf/avantgo.html">AvantGo
Synchronization with Pocket PC</a> provided by <a
 href="mailto:JudgeBeavis_at_hotmail_dot_com">Michael Jarrett</a>.
Note,
compilation of agsync-0.2-pre is enough - installation is not
required but of course allowed ;-)<br>
  </li>
</ul>
<h3>SynCE-KDE compilation and installation</h3>
<ol>
  <li>Download SynCE-KDE (synce-kde-0.x.y.tar.gz)&nbsp; from the <a
 href="http://sourceforge.net/projects/synce/">SourceForge
Project Page</a> or check out the latest code from the <a
 href="http://sourceforge.net/cvs/?group_id=30550">CVS
repository</a>.<br>
    <br>
  </li>
  <li>If you have downloaded from the <a
 href="http://sourceforge.net/projects/synce/">SourceForge Project
Page</a> go ahead to step 4.<br>
    <br>
  </li>
  <li>After checking out SynCE-KDE from CVS do a<br>
    <code>&nbsp;&nbsp;&nbsp; # make -f Makefile.cvs</code><br>
and go to step 5.<br>
    <br>
  </li>
  <li>Unpack the downloaded synce-kde-0.6.tar.gz into your favorite
source directory<br>
    <code>&nbsp;&nbsp;&nbsp; # tar -xzf
synce-kde-0.x.y.tar.gz</code><br>
    <br>
  </li>
  <li>Configure your SynCE-KDE source tree by issuing the
command<br>
    <code>&nbsp;&nbsp;&nbsp; # ./configure --prefix=`kde-config
--prefix`</code><br>
If you want the AvantGo synchronization plugin also to be compiled,
issue<br>
    <code>&nbsp;&nbsp;&nbsp; # ./configure --prefix=`kde-config
--prefix` \<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
--with-agsync=/path/to/agsync/source/dir/</code><br>
    <br>
  </li>
  <li>After configuring SynCE-KDE compile and install it by simply
doing a<br>
    <code>&nbsp;&nbsp;&nbsp; # make</code><br>
and as root<br>
    <code>&nbsp;&nbsp;&nbsp; # make install</code></li>
</ol>
<br>
<h2><a name="Related_Work" id="Related_Work"></a>Related Work</h2>
<ul>
  <li><a href="mailto:boris_at_brooknet_dot_com_dot_au">Sam
Lawrance</a> is
working
on porting SynCE and SynCE-KDE to <a href="http://www.freebsd.org/">FreeBSD</a>.
You will find the port on
his <a href="http://sam.stral.net/freebsd/synce.html">SynCE
on FreeBSD</a> page.</li>
</ul>
<br>
<h2><a name="Future" id="Future"></a>Future</h2>
Wait and see ;-)<br>
<p>Return to <a href="../index.php">SynCE main page</a>.</p>

<?php include 'footer.php'; ?>
