class Vcs

  # See http://rubyforge.org/projects/vcs
  # and http://vcs.rubyforge.org

  protocol_version '0.1'

  def local_commit! ( *args )
    common_commit!("k2.0 <%= rev %>: <%= title %>", *args) do |subject|
      mail!(:to => %w[kernelteam@gostai.com],
            :subject => subject)
    end
  end
  default_commit :local_commit

end # class Vcs
