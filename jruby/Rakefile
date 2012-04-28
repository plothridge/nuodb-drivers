require 'rake/clean'
CLEAN.include 'pkg', '**/pkg'

def rake(args)
  ruby "-S", "rake", *args
end

SUBDIRS = %w[jdbc-nuodb activerecord-jdbcnuodb-adapter]

task :default => ["all:build"]

task "all:build"   => [*SUBDIRS.map { |f| "#{f}:build" }]

task "all:install" => [*SUBDIRS.map { |f| "#{f}:install" }]

task "all:release" => [*SUBDIRS.map { |f| "#{f}:release" }]

(SUBDIRS).each do |subdir|
  namespace subdir do

    task :build do
      puts "----- #{subdir}:build"
      Dir.chdir(subdir) do
        rake "build"
      end
      Dir.mkdir("pkg") unless File.directory?("pkg")
      cp FileList["#{subdir}/pkg/#{subdir}-*.gem"], "pkg"
    end

    # bundler handles install => build itself
    task :install do
      puts "----- #{subdir}:install"
      Dir.chdir(subdir) do
        rake "install"
      end
    end

    task :release do
      puts "----- #{subdir}:release"
      Dir.chdir(subdir) do
        rake "release"
      end
    end
  end
end

