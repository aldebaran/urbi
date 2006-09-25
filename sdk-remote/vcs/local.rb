class Vcs

  # See http://rubyforge.org/projects/vcs
  # and http://vcs.rubyforge.org

  protocol_version '0.1'

  def local_commit! ( *args )
    common_commit!("liburbi-c++ <%= rev %>: <%= title %>", *args) do |subject|
      mail!(:to => %w[akim@lrde.epita.fr
                      matthieu.nottale@ensta.fr
                      baillie@gostai.com],
            :subject => subject)
    end
  end
  default_commit :local_commit

end # class Vcs
