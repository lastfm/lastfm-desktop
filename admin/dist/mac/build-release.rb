### Usage build-release.rb --deltas <version> <version> [--no-build] [--no-package]
# the deltas should be in a zip in the filder under bin with the expected filename format
# example /_bin/<version>/Last.fm-<version>
# TODO: download the previous versions from cdn and use that for the delta
#       there's no chance they'll be wrong then

require 'rexml/document'

# find the app version from the app's Info.plist
xml_data = File::read( "admin/dist/mac/Standard.plist" )

# extract event information
doc = REXML::Document.new(xml_data)

doc.elements.each('plist/dict/key') do |ele|
	if ele.text == "CFBundleVersion"
		$version = ele.next_element().text
	end
end

print "Building version #{$version}\n"

# get the delta versions from the command line
$deltas = []
deltasIndex = ARGV.index("--deltas")

if deltasIndex != nil
	while deltasIndex + 1 < ARGV.length and not ARGV.at( deltasIndex + 1 ).start_with?("--")
		deltasIndex = deltasIndex + 1
		print "Creating binary delta for version #{ARGV.at(deltasIndex)}\n"
		$deltas << ARGV.at(deltasIndex)
	end
end



$upload_folder = '/userhome/michael/www/client/Mac'
$download_folder = 'http://users.last.fm/~michael/client/Mac'

## Check that we are running from the root of the lastfm-desktop project
# ?

def clean
	## clean everything
	system 'make clean'
	system 'make distclean'
end

def build
	## build everything
	Dir.chdir("../liblastfm") do
		system "rm -rf _release"
		Dir.mkdir("_release")
		Dir.chdir("_release") do
			system 'cmake -D CMAKE_BUILD_TYPE:STRING=Release ..'
			system 'sudo make install'
		end
	end

	system 'qmake -r CONFIG+=release'
	system 'make'

	system 'rm -rf _bin/Last.fm.app'
	system 'mv "_bin/Last.fm Scrobbler.app" _bin/Last.fm.app'
end

def copy_plugin
	## copy the iTunes plugin into the bundle
	system "cp -R _bin/Audioscrobbler.bundle '_bin/Last.fm.app/Contents/MacOS/'"
end

def create_zip
	## create a zip file
	Dir.chdir("_bin") do
		system "rm -rf #{$version}"
		system "mkdir #{$version}"
		system "tar cjf #{$version}/Last.fm-#{$version}.tar.bz2 'Last.fm.app'"
		Dir.chdir("Last.fm.app/Contents") do
			system "tar cjf ../../#{$version}/Last.fm_Mac_Update_#{$version}.tar.bz2 *"
		end
		system "zip -ry #{$version}/Last.fm-#{$version}.zip 'Last.fm.app'"
	end
end

def create_deltas
	## create any deltas
	Dir.chdir("_bin") do
		# unzip the new app
		puts "unzipping #{$version}"
		system "unzip -q #{$version}/Last.fm-#{$version}.zip -d #{$version}"

		$deltas.each do |delta|
			# unzip the old version (try both compression formats)
			puts "unzipping #{delta}"
			#system 'tar -xjf Last.fm-#{delta}.tar.bz2 -C #{delta}'
			system "unzip -q #{delta}/Last.fm-#{delta}.zip -d #{delta}"
			# create the delta
			system "./BinaryDelta create #{delta}/Last.fm.app #{$version}/Last.fm.app #{$version}/Last.fm-#{$version}-#{delta}.delta"
			# check that it worked
			system "./BinaryDelta apply #{delta}/Last.fm.app #{$version}/Last.fm.app #{$version}/Last.fm-#{$version}-#{delta}.delta"
			# remove the unzipped old version
			system "rm -rf #{delta}/Last.fm.app"
		end

		# delete the unzipped new app
		system "rm -rf #{$version}/Last.fm.app"
	end
end

def upload_files
	# scp the main zip file
	# scp all the deltas
	# put them in my userhome if we are doing a test update

	system "scp _bin/#{$version}/Last.fm-#{$version}.tar.bz2 badger:#{$upload_folder}"
	system "scp _bin/#{$version}/Last.fm_Mac_Update_#{$version}.tar.bz2 badger:#{$upload_folder}"
	system "scp _bin/#{$version}/Last.fm-#{$version}.zip badger:#{$upload_folder}"

	$deltas.each do |delta|
		system "scp _bin/#{$version}/Last.fm-#{$version}-#{delta}.delta badger:#{$upload_folder}"
	end
end

def generate_appcast_xml
	## sign the zip file and deltas

	File.open("update.xml", 'w') do |f|

		item = "<item>\n"
		item << "\t<title>#{$version}</title>\n"
		item << "\t<pubDate></pubDate>\n"

		version_sig = `ruby admin/dist/mac/sign_update.rb _bin/#{$version}/Last.fm-#{$version}.tar.bz2 admin/dist/mac/dsa_priv.pem`.strip
		version_size = `du _bin/#{$version}/Last.fm-#{$version}.tar.bz2`.split[0]

		item << "\t<enclosure\n\t\tsparkle:version=\"#{$version}\"\n\t\turl=\"#{$download_folder}/Last.fm-#{$version}.tar.bz2\"\n\t\tlength=\"#{version_size}\"\n\t\ttype=\"application/octet-stream\"\n\t\tsparkle:dsaSignature=\"#{version_sig}\"/>\n"
		item << "\t<sparkle:deltas>\n"

		$deltas.each do |delta|
			delta_sig = `ruby admin/dist/mac/sign_update.rb _bin/#{$version}/Last.fm-#{$version}-#{delta}.delta admin/dist/mac/dsa_priv.pem`.strip
			delta_du = `du _bin/#{$version}/Last.fm-#{$version}-#{delta}.delta`.split[0]
			item << "\t\t<enclosure\n\t\t\turl=\"#{$download_folder}/Last.fm-#{$version}-#{delta}.delta\"\n\t\t\tsparkle:version=\"#{$version}\"\n\t\t\tsparkle:deltaFrom=\"#{delta}\"\n\t\t\tlength=\"#{delta_du}\"\n\t\t\ttype=\"application/octet-stream\"\n\t\t\tsparkle:dsaSignature=\"#{delta_sig}\"/>\n"
		end

		item << "\t</sparkle:deltas>\n"
		item << "\t<description></description>\n"
		item << "</item>\n"

		f.syswrite item
	end
end

if not ARGV.include?( "--no-build" )
	# build the app
	clean
	build
	copy_plugin
end
if not ARGV.include?( "--no-package" )
	# package and upload the app
	create_zip
	create_deltas
	upload_files
	generate_appcast_xml
end

