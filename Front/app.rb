# coding: utf-8
require "sinatra/base"
require "open3"
require "json"

class BasicConfig < Sinatra::Base
  set :public_folder, File.dirname(__FILE__) + "/assets"
end

class MainApp < Sinatra::Base
  helpers do
    include Rack::Utils
    alias_method :h, :escape_html
  end

  get '/' do
    erb :index
  end

  post '/eval' do
    code = params[:code]
    out, err, s = Open3.capture3("./bin/SliP", :stdin_data => code)
    JSON.generate({
      :out => out,
      :err => err,
    })
  end
end

class Application < Sinatra::Base
  use BasicConfig
  use MainApp
  run!({:bind => '0.0.0.0', :port => 8080}) if app_file == $0
end

