
websequencediagram for job flow

    main->job_generator: start
    job_generator->job_generator: prepare_frame
    job_generator->job_storage: num_jobs ?
    job_storage->job_generator: N
    job_generator->job_storage: add_job *
    main->renderer: start
    renderer->renderer: render_frame
    renderer->job_storage: get_job
    job_storage->renderer: <JOB>
    renderer->worker: <JOB>
    worker->renderer: job_ready
    renderer->job_storage: remove_job
    renderer->streamer: frame **
    streamer->streamer: combine
    streamer->output: video frame
